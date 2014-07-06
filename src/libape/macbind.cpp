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

//#include <iostream>
//using namespace std;

#define BUILD_CROSS_PLATFORM
#include <All.h>
#include <MACLib.h>
#include <APETag.h>
#include <APEInfo.h>
#include <CharacterHelper.h>

#include "macbind.h"
//#include "vfscio.h"

VFSCIO::VFSCIO()
{
	vfs = NULL;
	readfunc = NULL;
	seekfunc = NULL;
	openfunc = NULL;
	closefunc = NULL;
	ftellfunc = NULL;
	opened = false;
}
VFSCIO::~VFSCIO()
{
	Close();
}

int VFSCIO::Open(const wchar_t* pName, BOOL bOpenReadOnly)
{
	if(openfunc)
	{
		vfs = (*openfunc)((char*)(CAPECharacterHelper::GetUTF8FromUTF16(pName)),"rb");
		if(vfs)return true;
	}
	return false;
}

int VFSCIO::Close()
{
	if(opened && vfs)
	{
		return (*closefunc)(vfs);
	}
	return true;
}

int VFSCIO::setupvfsinterface(	int64_t	(*sreadfunc) (void *, int64_t, int64_t, void*),\
		int		(*sseekfunc) (void*, int64_t, int),\
		void*	(*sopenfunc) (const char *, const char *),\
		int 	(*sclosefunc)(void*),\
		int64_t (*sftellfunc)(void*))
{
	readfunc = sreadfunc;
	seekfunc = sseekfunc;
	openfunc = sopenfunc;
	closefunc = sclosefunc;
	ftellfunc = sftellfunc;
	return true;
}

int VFSCIO::Read(void * pBuffer, unsigned int nBytesToRead, unsigned int * pBytesRead)
{
	if(readfunc && vfs!=0)
	{
		*pBytesRead = (*readfunc)(pBuffer, 1, nBytesToRead, vfs);
/*		cout << "bytes to read " <<  nBytesToRead << " bytes read " << *pBytesRead << " ";
		for(int i=0;i<*pBytesRead;i++)
			cout << ((char*)pBuffer)[i] << " ";
		cout << "\n";
*/		return 0;
	}
	return 1;
}

int VFSCIO::Seek(int nDistance, unsigned int nMoveMode)
{
	if(seekfunc!=0 && vfs!=0)
	{
		return (*seekfunc)(vfs,(int64_t)nDistance,nMoveMode);
	}
	return 1;
}

int VFSCIO::Write(const void * pBuffer, unsigned int nBytesToWrite, unsigned int * pBytesWritten)
{
	return 1;
}

int VFSCIO::SetEOF()
{
	return false;
}

// creation / destruction
int VFSCIO::Create(const wchar_t * pName)
{
//	return (*openfunc)();
	return false;
}

int VFSCIO::Delete()
{
	return false;
}

// attributes
int VFSCIO::GetPosition()
{
	if(ftellfunc)
	{
		return (*ftellfunc)(vfs);
	}
	return -1;
}

int VFSCIO::GetSize()
{
    int nCurrentPosition = GetPosition();
    Seek(0, FILE_END);
    int nLength = GetPosition();
    Seek(nCurrentPosition, FILE_BEGIN);
    return nLength;
}

int VFSCIO::GetName(wchar_t * pBuffer)
{
	return false;
}

void VFSCIO::setvfs(void *svfs)
{
	if(opened)
	{
		Close();
		opened = false;
	}
	vfs = svfs;
}

VFSCIO vfsclass;

void deleteAPED(void* pAPEDecompress)
{
	delete ((IAPEDecompress*)pAPEDecompress);
}

void* createAPED(const char * pFilename, int * pErrorCode = NULL)
{
	return (void*)(CreateIAPEDecompress(CAPECharacterHelper::GetUTF16FromUTF8((str_utf8*) pFilename), pErrorCode));
}

void* createAPEDvfs(void* vfs, int * pErrorCode = NULL)
{
	vfsclass.setvfs(vfs);
	return (void*)(CreateIAPEDecompressEx(&vfsclass, pErrorCode));
}

int GetInfoTotalBlocksAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS);
}

int GetInfoBlockAlignAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_INFO_BLOCK_ALIGN);
}

int GetDataAPED(void* pAPEDecompress,char * pBuffer, int nBlocks, int * pBlocksRetrieved)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetData(pBuffer,nBlocks,pBlocksRetrieved);
}

int SeekAPED(void* pAPEDecompress, int nBlocks)
{
	return ((IAPEDecompress*)pAPEDecompress)->Seek(nBlocks);
}


int GetInfoAverageBitRateAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_DECOMPRESS_AVERAGE_BITRATE);
}

int GetInfoLengthMsAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_INFO_LENGTH_MS);
}

int GetInfoSampleRateAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_INFO_SAMPLE_RATE);
}

int GetInfoChannelsAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_INFO_CHANNELS);
}

int GetInfoBitsPerSampleAPED(void* pAPEDecompress)
{
	return ((IAPEDecompress*)pAPEDecompress)->GetInfo(APE_INFO_BITS_PER_SAMPLE);
}

int SetUpVfsInterface(	int64_t	(*sreadfunc) (void *, int64_t, int64_t, void*),\
		int		(*sseekfunc) (void*, int64_t, int),\
		void*	(*sopenfunc) (const char *, const char *),\
		int 	(*sclosefunc)(void*),\
		int64_t (*sftellfunc)(void*) )
{
	return vfsclass.setupvfsinterface(sreadfunc, sseekfunc, sopenfunc, sclosefunc, sftellfunc);
}

void SetVfs(void *svfs)
{
	vfsclass.setvfs(svfs);
}
