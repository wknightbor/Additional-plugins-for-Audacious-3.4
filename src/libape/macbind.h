/*
 * Copyright (c) 2014 Andrey Karpenko <andrey@delfa.net>.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MACBIND_H_
#define MACBIND_H_

#include <sys/types.h>

#ifdef __cplusplus
class VFSCIO : public CIO
{
public:

    // construction / destruction
	VFSCIO();
    ~VFSCIO();

    // open / close
    int Open(const wchar_t* pName, BOOL bOpenReadOnly = false);
    int Close();

    // read / write
    int Read(void * pBuffer, unsigned int nBytesToRead, unsigned int * pBytesRead);
    int Write(const void * pBuffer, unsigned int nBytesToWrite, unsigned int * pBytesWritten);

    // seek
    int Seek(int nDistance, unsigned int nMoveMode);

    // other functions
    int SetEOF();

    // creation / destruction
    int Create(const wchar_t * pName);
    int Delete();

    // attributes
    int GetPosition();
    int GetSize();
    int GetName(wchar_t * pBuffer);

    int setupvfsinterface(	int64_t	(*sreadfunc) (void *, int64_t, int64_t, void*),\
    						int		(*sseekfunc) (void*, int64_t, int),\
    						void*	(*sopenfunc) (const char *, const char *),\
    						int 	(*sclosefunc)(void*),\
    						int64_t (*sftellfunc)(void*) );
    void setvfs(void* svfs);
private:
    int64_t (*readfunc) (void *, int64_t, int64_t, void*);
    int 	(*seekfunc) (void*, int64_t, int);
    void* 	(*openfunc) (const char * path, const char * mode);
    int 	(*closefunc)(void* file);
    int64_t	(*ftellfunc)(void* file);
    void * vfs;
    bool 	opened;
};
#endif

#ifdef __cplusplus
extern "C"
{
#endif

void deleteAPED(void* pAPEDecompress);
void* createAPED(const char * pFilename, int * pErrorCode);
void* createAPEDvfs(void* vfs, int * pErrorCode);
int GetInfoTotalBlocksAPED(void* pAPEDecompress);
int GetInfoBlockAlignAPED(void* pAPEDecompress);
int GetDataAPED(void* pAPEDecompress,char * pBuffer, int nBlocks, int * pBlocksRetrieved);
int GetInfoAverageBitRateAPED(void* pAPEDecompress);
int GetInfoLengthMsAPED(void* pAPEDecompress);
int GetInfoSampleRateAPED(void* pAPEDecompress);
int GetInfoChannelsAPED(void* pAPEDecompress);
int GetInfoBitsPerSampleAPED(void* pAPEDecompress);
int SeekAPED(void* pAPEDecompress, int nBlocks);


int SetUpVfsInterface(	int64_t	(*sreadfunc) (void *, int64_t, int64_t, void*),\
		int		(*sseekfunc) (void*, int64_t, int),\
		void*	(*sopenfunc) (const char *, const char *),\
		int 	(*sclosefunc)(void*),\
		int64_t (*sftellfunc)(void*) );
void SetVfs(void *svfs);

#ifdef __cplusplus
}
#endif

#endif /* MACBIND_H_ */
