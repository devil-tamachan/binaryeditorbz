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

#import "Document.h"
#include <sys/stat.h>

#define MAXONMEMORY 1024*1024
#define MAXMAPSIZE 64*1024*1024
#define READONLY true


@implementation Document

static BOOL g_isNewWindow = TRUE;

+ (void)setIsNewWindow:(BOOL)isNewWindow
{
    g_isNewWindow = isNewWindow;
}
+ (BOOL)isNewWindow
{
    return g_isNewWindow;
}

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        NSLog(@"Document::init()");
        m_pData = NULL;
        m_dwTotal = 0;
        m_bReadOnly = READONLY;
        //m_hMapping = NULL;
        m_pFileMapping = 0;
        m_pMapStart = 0;
        m_dwFileOffset = 0;
        m_dwMapSize = 0;
        m_dwBase = 0;
        
        m_pagingSize = sysconf(_SC_PAGESIZE);
    }
    return self;
}

/*
- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"Document";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}
 */

-(BZWindow *)GetActiveBZWindow
{
    NSUInteger max = [[NSApp windows] count];
    for (NSUInteger i = 0; i<max; i++) {
        NSWindow *target = [[NSApp windows] objectAtIndex:i];
        if([target isKeyWindow] && [target isKindOfClass:[BZWindow class]])
            return target;
    }
    return nil;
}

- (void)makeWindowControllers
{
    NSWindowController *activeWindowController = nil;
    if (!g_isNewWindow) {
        BZWindow *activeWindow = [self GetActiveBZWindow];
        if (activeWindow) activeWindowController = activeWindow.windowController;
    }
    if (activeWindowController) {
        //recycle window
        [self addWindowController:activeWindowController];
    } else {
        //new window
        BZWindowController *mainWindowController = [[BZWindowController alloc] initWithWindowNibName:@"BZWindowController"];
        [self addWindowController:mainWindowController];
    }
}

+ (BOOL)autosavesInPlace
{
    return NO;//YES;
}

- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError *__autoreleasing *)outError
{
    return [self OnSaveDocument:url];
}

- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError *__autoreleasing *)outError
{
    BOOL ret = [self OnOpenDocument:url];
    if(ret)
    {
        //NSWindow *nswnd = [[[self windowControllers] objectAtIndex:0] window];
        //BZWindow *wnd = _window;
        //[_bzwnd1 CreateClient];
    }
    return ret;
}

-(__uint8_t *)GetDocPtr
{
    return m_pData;
}

-(__uint64_t)GetDocSize
{
    return m_dwTotal;
}

- (void)DeleteContents
{
    if (m_pData) {
        if ([self IsFileMapping]) {
            munmap(m_pMapStart ? m_pMapStart : m_pData, m_dwMapSize);
            m_pMapStart = NULL;
            m_dwFileOffset = 0;
            m_dwMapSize = 0;
        } else {
            free(m_pData);
        }
        m_pData = NULL;
        m_dwTotal = 0;
        m_dwBase = 0;
        /* UpdateAllViews(NULL);*/
    }
    if ([self IsFileMapping]) {
        if (m_pDupDoc) {
            m_pDupDoc->m_pDupDoc = nil;
            m_pDupDoc = nil;
            //m_hMapping = NULL;
            m_pFileMapping = 0;
        } else {
            if (m_pFileMapping) {
                close(m_pFileMapping);
                m_pFileMapping = 0;
            }
        }
    }
    
    /*if (m_pUndo) {
     free(m_pUndo);
     m_pUndo = NULL;
     }*/
    m_bReadOnly = false;
}

-(BOOL)IsFileMapping
{
    return m_pFileMapping != 0;
}

- (BOOL)MapView
{
    m_dwMapSize = m_dwTotal;
    m_pData = mmap(NULL, m_dwMapSize, m_bReadOnly?PROT_READ : PROT_READ|PROT_WRITE, MAP_SHARED, m_pFileMapping, 0);
    if (m_pData==MAP_FAILED) {//Failed
        m_dwMapSize = MAXMAPSIZE;
        m_pData = mmap(NULL, m_dwMapSize, m_bReadOnly?PROT_READ : PROT_READ|PROT_WRITE, MAP_SHARED, m_pFileMapping, 0);
        if (m_pData!=MAP_FAILED) {//Success
            m_pMapStart = m_pData;
            m_dwFileOffset = 0;
        } else {
            return false;
        }
    }
    return true;
}

-(BOOL)IsOutOfMap1:(__uint8_t *)p
{
    return (p < m_pMapStart || p >= (m_pMapStart + m_dwMapSize));
}

- (__uint64_t)GetMapSize
{
    return m_pMapStart ? m_dwFileOffset+m_dwMapSize : m_dwTotal;
}

-(__uint8_t *)QueryMapView:(__uint8_t *)pBegin dwOffset:(__uint64_t)dwOffset
{
    __uint8_t *p = pBegin + dwOffset;
    if (m_pMapStart && [self IsOutOfMap1:p])
        return [self QueryMapView1:pBegin dwOffset:dwOffset];
    else return pBegin;
}

-(__uint8_t *)QueryMapView1:(__uint8_t *)pBegin dwOffset:(__uint64_t)dwOffset
{
    __uint8_t *p = pBegin + dwOffset;
    
    if (p == m_pData + m_dwTotal && p == m_pMapStart + m_dwMapSize)
        return pBegin;
    
    __uint64_t dwBegin = [self GetFileOffsetFromFileMappingPointer:pBegin];
    munmap(m_pMapStart, m_dwMapSize);
    uint64_t dwTmp1 = [self GetFileOffsetFromFileMappingPointer:p];
    m_dwFileOffset = dwTmp1 - (dwTmp1 % m_pagingSize);
    m_dwMapSize = MIN(MAXMAPSIZE, m_dwTotal - m_dwFileOffset);
    if (m_dwMapSize == 0) {
        dwTmp1 = (m_dwTotal - m_pagingSize);
        m_dwFileOffset = dwTmp1 - (dwTmp1 % m_pagingSize);
        m_dwMapSize = m_dwTotal - m_dwFileOffset;
    }
    m_pMapStart = mmap(NULL, m_dwMapSize, m_bReadOnly?PROT_READ : PROT_READ|PROT_WRITE, MAP_SHARED, m_pFileMapping, m_dwFileOffset);
    if (!m_pMapStart) {
        NSLog(@"Error: Document::QueryMapView1");
        return NULL;
    }
    m_pData = m_pMapStart - m_dwFileOffset;
    pBegin = [self GetFileMappingPointerFromFileOffset:dwBegin];
    return pBegin;
}

-(__uint64_t)GetFileOffsetFromFileMappingPointer:(__uint8_t *)pConv
{
    return pConv - m_pData;
}

-(__uint8_t *)GetFileMappingPointerFromFileOffset:(__uint64_t)dwFileOffset
{
    return dwFileOffset + m_pData;
}

-(BOOL)OnOpenDocument:(NSURL *)url
{
    int fd = open([[url path] fileSystemRepresentation], O_RDWR|O_EXLOCK/*|O_NONBLOCK*/);
    if (fd==-1) {
        m_bReadOnly = true;
    }
    close(fd);
    
    fd = open([[url path] fileSystemRepresentation], O_RDONLY|O_EXLOCK/*|O_NONBLOCK*/);
    if(fd==-1)
    {
        return false;//err
    }
    //    flock(fd, LOCK_EX|LOCK_NB);
    
    [self DeleteContents];
    
    off_t fsize = [BZGlobalFunc GetFileLength:fd];
    if(fsize<0)
    {
        close(fd);
        return false;//err
    }
    
    if (fsize >= MAXONMEMORY) { //file-mpping
        /*struct stat st;
         if(fstat(fd, &st)==-1)
         {
         return false;//err
         }*/
        //m_bReadOnly = /*options.bReadOnly||*/ st.mode;
        if (m_bReadOnly==false) {
            close(fd);
            fd = open([[url path] fileSystemRepresentation], O_RDWR|O_EXLOCK/*|O_NONBLOCK*/);
            if (fd == -1) {
                return false;//err
            }
        }
        m_pFileMapping = fd;
    }
    
    m_dwTotal = fsize;
    
    if ([self IsFileMapping]) {
        if([self MapView]==false) {
            close(fd);
            return false;//err
        }
    } else {
        if (!(m_pData = malloc(m_dwTotal))) {
            close(fd);
            return false;
        }
        ssize_t totalRead = read(fd, m_pData, m_dwTotal);
        if (totalRead < m_dwTotal) {
            free(m_pData);
            m_pData = NULL;
            close(fd);
            return false;//err
        }
        m_bReadOnly = READONLY;
    }
    
    //munmap(data, m_dwMapSize);
    close(fd);
    
    return true;
}

- (BOOL)OnSaveDocument:(NSURL *)url
{
    BOOL bResult = FALSE;
    if([self IsFileMapping]) {
        int ret = msync(m_pMapStart ? m_pMapStart : m_pData, m_dwMapSize, MS_SYNC);
        bResult = (ret==0);//(m_pMapStart) ? ::FlushViewOfFile(m_pMapStart, m_dwMapSize) : ::FlushViewOfFile(m_pData, m_dwTotal);
        if(!bResult) {
            NSAlert *al = [[NSAlert alloc] init];
            [al setMessageText:NSLocalizedString(@"msync error", nil)];
            [al runModal];
        }
    } else {
        int fd = open([[url path] fileSystemRepresentation], O_WRONLY|O_CREAT|O_TRUNC|O_EXLOCK);
        if (fd!=-1)
        {
            ssize_t writed = write(fd, m_pData, m_dwTotal);//pFile->Write(m_pData, m_dwTotal);
            close(fd);
            bResult = (writed==m_dwTotal);
        }
    }
    if (bResult)
    {
        //m_dwUndoSaved = m_dwUndo;		// ###1.54
        //TouchDoc();
        return YES;
    }
    return NO;
}

- (void)TouchDoc
{
	//SetModifiedFlag(m_dwUndo != m_dwUndoSaved);
	//GetMainFrame()->OnUpdateFrameTitle();
}


- (Document *)GetBrotherDoc
{
    for (NSWindowController *wcon in self.windowControllers) {
        BZWindowController *bzwcon = (BZWindowController*)wcon;
        if(bzwcon->m_doc1 == self) {
            return bzwcon->m_doc2;
        } else if(bzwcon->m_doc2 == self) {
            return bzwcon->m_doc1;
        }
    }
    return nil;
}

- (BOOL)isDocumentEditedSelfOnly
{
    return m_dwUndoSaved != m_dwUndo;
}

- (BOOL)isDocumentEdited
{
    if ([self isDocumentEditedSelfOnly]) {
        return YES;
    }
    Document *brotherDoc = [self GetBrotherDoc];
    if (brotherDoc && [brotherDoc isDocumentEditedSelfOnly]) {
        return YES;
    }
    return NO;
}


- (void)InsertData:(__uint64_t)dwPtr dwSize:(__uint64_t)dwSize bIns:(BOOL)bIns
{
	BOOL bGlow = false;
	__uint64_t nGlow = dwSize - (m_dwTotal - dwPtr);
	if(nGlow <= dwSize/*overflow check*/ && nGlow > 0)bGlow=true;
	if(!m_pData) {
		m_pData = (__uint8_t *)malloc(dwSize);
		m_dwTotal = dwSize;
	} else if(bIns || dwPtr == m_dwTotal) {
        m_pData = (__uint8_t *)realloc(m_pData, m_dwTotal+dwSize);
        memmove(m_pData+dwPtr+dwSize, m_pData+dwPtr, m_dwTotal - dwPtr);
        m_dwTotal += dwSize;
	} else if(bGlow) {
        m_pData = (__uint8_t *)realloc(m_pData, m_dwTotal+nGlow);
        m_dwTotal += nGlow;
	}
	//ASSERT(m_pData != NULL);
}

- (void)DeleteData:(__uint64_t)dwPtr dwSize:(__uint64_t)dwSize
{
	if(dwPtr == m_dwTotal) return;
	memmove(m_pData+dwPtr, m_pData+dwPtr+dwSize, m_dwTotal-dwPtr-dwSize);
	m_dwTotal -= dwSize;
    
	if(![self IsFileMapping])
		m_pData = (__uint8_t *)realloc(m_pData, m_dwTotal);
	[self TouchDoc];
}


//dwBlockは構造体全体のサイズ
// if mode==UNDO_OVR
// 8 (__uint64_t)dwPtr
// 1 (__uint8_t)(enum UndoMode)mode
// dwSize data 
// 8 (__uint64_t)dwBlock(==dwSize+sizeof(__uint64_t)*2+sizeof(__uint8_t))

// if mode==UNDO_DEL
// 8 (__uint64_t)dwPtr
// 1 (__uint8_t)(enum UndoMode)mode
// 8 dwSize
// 8 (__uint64_t)dwBlock(==dwSize+sizeof(__uint64_t)*2+sizeof(__uint8_t))

- (void)StoreUndo:(__uint64_t)dwPtr dwSize:(__uint64_t)dwSize mode:(enum UndoMode)mode
{
	if(mode == UNDO_OVR && dwPtr+dwSize >= m_dwTotal)
		dwSize = m_dwTotal - dwPtr;
	if(dwSize == 0) return;
    
	[self QueryMapView:m_pData dwOffset:dwPtr];
	
    __uint64_t dwBlock = dwSize +sizeof(__uint64_t)*2+sizeof(__uint8_t);
	if(mode == UNDO_DEL)
		dwBlock = sizeof(__uint64_t) +sizeof(__uint64_t)*2+sizeof(__uint8_t);
	if(!m_pUndo) {
		m_pUndo = (__uint8_t*)malloc(dwBlock);
		m_dwUndo = m_dwUndoSaved = 0;
	} else
		m_pUndo = (__uint8_t*)realloc(m_pUndo, m_dwUndo+dwBlock);
	//ASSERT(m_pUndo != NULL);
	__uint8_t *p = m_pUndo + m_dwUndo;
	*((__uint64_t *)p) = dwPtr;
    p+=8;
	*p++ = mode;
	if(mode == UNDO_DEL) {
		*((__uint64_t *)p) = dwSize;
        p+=8;
	} else {
		memcpy(p, m_pData+dwPtr, dwSize);
		p+=dwSize;
	}
	*((__uint64_t *)p) = dwBlock;
    p+=8;
	m_dwUndo += dwBlock;
	//ASSERT(p == m_pUndo+m_dwUndo);
	[self TouchDoc];
}

- (__uint64_t)DoUndo
{
	__uint64_t dwSize = *((__uint64_t *)(m_pUndo+m_dwUndo-sizeof(__uint64_t)));
	m_dwUndo -= dwSize;
	dwSize -= sizeof(__uint64_t)*2+sizeof(__uint8_t);
	__uint8_t *p = m_pUndo + m_dwUndo;
	__uint64_t dwPtr = *((__uint64_t *)p);
    p+=8;
	enum UndoMode mode = (enum UndoMode)(*p++);
	
    [self QueryMapView:m_pData dwOffset:dwPtr];//QueryMapView(m_pData, dwPtr);
    
	if(mode == UNDO_DEL) {
		[self DeleteData:dwPtr dwSize:*((__uint64_t *)p)];//DeleteData(dwPtr, *((DWORD*)p));
	} else {
		[self InsertData:dwPtr dwSize:dwSize bIns:mode == UNDO_INS];
		memcpy(m_pData+dwPtr, p, dwSize);
	}
	if(m_dwUndo)
		m_pUndo = (__uint8_t *)realloc(m_pUndo, m_dwUndo);
	else {				// ### 1.54
		free(m_pUndo);
		m_pUndo = NULL;
		if(m_dwUndoSaved)
			m_dwUndoSaved = UINT_MAX;
	}
	// if(!m_pUndo)
    [self TouchDoc];//TouchDoc();
	return dwPtr;
}


-(void)CloseDocument
{
    NSLog(@"Document::CloseDocument");
    [self DeleteContents];
}

/*-(void)close
 {
 NSLog(@"close");
 //    [self CloseDocument]; iranai ppoi
 [super close];
 }*/

-(void)dealloc
{
    NSLog(@"Doument::dealloc");
}


@end
