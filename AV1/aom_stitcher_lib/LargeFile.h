/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: LargeFile.h
*
* Authors: Yong-Hwan Kim <yonghwan@keti.re.kr>
*          Sungjei Kim <sungjei.kim@keti.re.kr>
*
* Original: "LargeFile.h" created by yonghwan@keti.re.kr, 2010.01.22
*           which is modified from JSVM 9.19.3
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#pragma once

#include <list>
#include <string>

#define	MSYS_WIN32

#if defined( MSYS_LINUX )
# if (!defined( MSYS_UNIX_LARGEFILE )) || (!defined(_LARGEFILE64_SOURCE) )
#  error Large file support requires MSYS_UNIX_LARGEFILE and _LARGEFILE64_SOURCE defined
# endif
#endif

class LargeFile
{
public:
	enum OpenMode
	{
		OM_READONLY,
		OM_WRITEONLY,
		OM_APPEND,
		OM_READWRITE
	};

public:
	LargeFile();
	~LargeFile();

	bool		open( const std::string& rcFilename, enum OpenMode eOpenMode, int iPermMode=0777 );
	bool		is_open() const		{	return (m_pFileHandle ? true : false);		}
	bool		close();
	bool		seek( int64_t iOffset, int iOrigin );
	int64_t		tell();

	bool		read( void *pvBuffer, uint32_t uiCount, uint32_t& ruiBytesRead );
	bool		write( const void *pvBuffer, uint64_t uiCount );

	FILE*		getFileHandle()		{	return m_pFileHandle;			}

private:
	FILE*		m_pFileHandle;
};


