//
//  Document.m
//  TamaBinary
//
//  Created by ad on 12/12/01.
//  Copyright (c) 2012年 tamachanbinary. All rights reserved.
//

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

- (void)makeWindowControllers
{
    NSLog(@"makeWindowControllers");
    NSWindow *activeWindow = [self GetActiveWindow];
    if (!g_isNewWindow && activeWindow) {
        //recycle window
        [self addWindowController:activeWindow.windowController];
    } else {
        //new window
        BZWindowController *mainWindowController = [[BZWindowController alloc] initWithWindowNibName:@"BZWindowController"];
        [self addWindowController:mainWindowController];
    }
}

+ (BOOL)autosavesInPlace
{
    return YES;
}

//writefromurl ka savefromurl kamo?
- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
    @throw exception;
    return nil;
}

-(NSWindow *)GetActiveWindow
{
    NSUInteger max = [[NSApp windows] count];
    for (NSUInteger i = 0; i<max; i++) {
        NSWindow *target = [[NSApp windows] objectAtIndex:i];
        if([target isKeyWindow]) return target;
    }
    return nil;
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
    return true;
}

/*
- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
    // You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
    // If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
    NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
    @throw exception;
    return YES;
}
*/


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

/*
-(off_t)GetFileLength:(int)fd
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
 */

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
