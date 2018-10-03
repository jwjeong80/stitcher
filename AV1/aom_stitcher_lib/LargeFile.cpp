/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: LargeFile.cpp
*
* Authors: Yong-Hwan Kim <yonghwan@keti.re.kr>
*          Sungjei Kim <sungjei.kim@keti.re.kr>
*
* Original: "LargeFile.cpp" created by yonghwan@keti.re.kr, 2010.01.22
*           which is modified from JSVM 9.19.3
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#include "stdafx.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <share.h>

#include "LargeFile.h"

#define WIN32
#ifdef WIN32


#else	// __linux__

#ifdef __ANDROID__
#else
#define		fopen		fopen64
#endif

#endif	// WIN32

using namespace std;

LargeFile::LargeFile()
    : m_pFileHandle(NULL)
{
}

LargeFile::~LargeFile()
{
    if (m_pFileHandle) {
        fclose(m_pFileHandle);
    }
}

bool LargeFile::open(const std::string& rcFilename, enum OpenMode eOpenMode, int iPermMode)
{
    if (rcFilename.empty())
        return false;

    if (m_pFileHandle)
        return false;

    // _O_SEQUENTIAL	=> S
    // _O_BINARY		=> b

    if (eOpenMode == OM_READONLY) {
        m_pFileHandle = _fsopen(rcFilename.c_str(), "rbS", _SH_DENYWR);     //_O_RDONLY;
    }
    else if (eOpenMode == OM_WRITEONLY) {
        m_pFileHandle = _fsopen(rcFilename.c_str(), "wbS", _SH_DENYWR);     //_O_CREAT | _O_TRUNC | _O_WRONLY;
    }
    else if (eOpenMode == OM_APPEND) {
        m_pFileHandle = _fsopen(rcFilename.c_str(), "abS", _SH_DENYWR);     //_O_APPEND | _O_CREAT | _O_WRONLY
    }
    else if (eOpenMode == OM_READWRITE) {
        m_pFileHandle = _fsopen(rcFilename.c_str(), "w+bS", _SH_DENYWR);    //_O_CREAT | _O_RDWR
    }
    else {
        //Trace("LargeFile: Not supported mode!\n");
        assert(0);
        return false;
    }

    // check if file is really open
    if (NULL == m_pFileHandle) {
        //Trace("File64 open failure!\n");
        return false;
    }

    return true;
}

bool LargeFile::close()
{
    int iRetv;

    if (NULL == m_pFileHandle)
        return false;

    iRetv = fclose(m_pFileHandle);

    m_pFileHandle = NULL;

    return (iRetv == 0) ? true : false;
}


bool LargeFile::seek(int64_t iOffset, int iOrigin)
{
    /* iOrigin:
        - SEEK_SET: Beginning of the file.
        - SEEK_CUR: Current position of the file pointer.
        - SEEK_END: End of file.
        */

    if (NULL == m_pFileHandle)
        return false;

#ifdef WIN32

    int64_t	iNewOffset = _fseeki64(m_pFileHandle, (long)iOffset, iOrigin);

#elif __linux__

#ifdef __ANDROID__
    int64_t iNewOffset = fseek( m_pFileHandle, int(iOffset & 0xFFFFFFFF), iOrigin );
#else
    int64_t iNewOffset = fseeko64( m_pFileHandle, iOffset, iOrigin );
#endif
#endif

    return (iNewOffset == -1) ? false : true;
}


int64_t LargeFile::tell()
{
    if (NULL == m_pFileHandle)
        return -1;

    int64_t iOffset;

#ifdef WIN32

    iOffset = _ftelli64(m_pFileHandle);

#elif __linux__

#ifdef __ANDROID__
    iOffset = ftell( m_pFileHandle );
#else
    iOffset = ftello64( m_pFileHandle );
#endif
#endif

    return iOffset;		// -1: fail
}


bool LargeFile::read(void *pvBuffer, uint32_t uiCount, uint32_t& ruiBytesRead)
{
    size_t iRetv;

    if (NULL == m_pFileHandle)
        return false;
    if (0 == uiCount)
        return false;

    ruiBytesRead = 0;

    iRetv = fread(pvBuffer, sizeof(char), uiCount, m_pFileHandle);
    if (iRetv != (int)uiCount) {
        //need to handle partial reads before hitting EOF

        //If the function tries to read at end of file, it returns 0.
        //If the handle is invalid, or the file is not open for reading,
        //or the file is locked, the function returns -1 and sets errno to EBADF.
        if (iRetv > 0) {
            //partial reads are acceptable and return the standard success code. Anything
            //else must be implemented by the caller.
            ruiBytesRead = (uint32_t)iRetv;
            return true;
        }
        else if (iRetv == -1) {
            //Trace("File64 reading error: %d\n", errno);
            return false;
        }
        else if (iRetv == 0) {
            ruiBytesRead = (uint32_t)iRetv;	// by yonghwan
            return false;
            //return true;
        }
        else {
            //Trace("File64 reading error, unexpected return code" );
            return false;
        }
    }
    else {
        ruiBytesRead = uiCount;
    }

    if (iRetv != (int)uiCount)
        return false;

    return true;
}


bool LargeFile::write(const void *pvBuffer, uint64_t uiCount)
{
    size_t iRetv;

    if (NULL == m_pFileHandle)
        return false;
    if (0 == uiCount)
        return false;

    iRetv = fwrite(pvBuffer, sizeof(char), uiCount, m_pFileHandle);
    if (iRetv != (int)uiCount)
        return false;

    return true;
}