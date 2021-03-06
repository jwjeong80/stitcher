/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: Main.cpp
*
* Authors: Sungjei Kim <sungjei.kim@keti.re.kr>
*
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#include "stdafx.h"

#if _MSC_VER
#pragma warning(disable:4996) //#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <process.h>    // C runtime thread
#include <windows.h>

#include <signal.h>     // Ctrl-C handler
#include <errno.h>

#include "ETimer.h"
//#include "HEVCFile.h"


//#include "StitcherApi.h"
#include "AV1StitcherApi.h"
#if _DEBUG
#pragma comment(lib, "../Lib/KetiBStrStitcher-debug.lib")
#else
#pragma comment(lib, "../Lib/KetiBStrStitcher.lib")
#endif

#define AV1 1

#if AV1
#include "AV1File.h"
#endif

// definitions
#define MAX_STREAMS         440 /* 20x22 */
#define MAX_FILENAME_SIZE   512
#define MAX_DEC_BUF         (4*1024*1024)



/* Ctrl-C handler */
static volatile sig_atomic_t b_ctrl_c /* = 0 */;
static void sigint_handler(int)
{
    b_ctrl_c = 1;
}

// function declaration
void StartBStrStitching(uint32_t uiNumTileRows, uint32_t uiNumTileCols, char **cFileNames, bool bAnnexB);
uint32_t __stdcall	OnBStrStitchProc(void* pThis);	// worker thread function
//bool xWriteIntoBuffer(uint8_t* pOutBuf, uint64_t& ruiOutBufPos, AccessUnit* pOutAUs);

// structure definition
struct sThreadArg
{
    uint32_t uiNumTileRows;
    uint32_t uiNumTileCols;
    char **  cFileNames;
	bool     bAnnexB;
};

// function definitions
void showHelp()
{
    fprintf(stdout, "\nUsage:\nstitcher.exe outstream numTileRows numTileCols stream[0] stream[1] ... stream[n]\n");
    fprintf(stdout, "Please check the input parameters!!\n");

    exit(1);
}

static int UnicodeToMultiByte(wchar_t* strIn, char* strOut, uint32_t uiOutBufLen)
{
    int ret = WideCharToMultiByte(CP_OEMCP, 0, strIn, -1, strOut, uiOutBufLen, NULL, NULL);
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{

#if AV1
	//argv[1] = L"..\\Bin\\outstream_2.av1";
	//argv[2] = L"3";             //numTileRows
	//argv[3] = L"3";             //numTileCols
	//argv[4] = L"..\\Bin\\input_3x3_0.av1";
	//argv[5] = L"..\\Bin\\input_3x3_1.av1";
	//argv[6] = L"..\\Bin\\input_3x3_2.av1";
	//argv[7] = L"..\\Bin\\input_3x3_3.av1";
	//argv[8] = L"..\\Bin\\input_3x3_4.av1";
	//argv[9] = L"..\\Bin\\input_3x3_5.av1";
	//argv[10] = L"..\\Bin\\input_3x3_6.av1";
	//argv[11] = L"..\\Bin\\input_3x3_7.av1";
	//argv[12] = L"..\\Bin\\input_3x3_8.av1";
	//argc = 12 + 1;
	argv[1] = L"..\\Bin\\outstream_2.obu";
	argv[2] = L"3";             //numTileRows
	argv[3] = L"3";             //numTileCols
	argv[4] = L"0";              //annexB flag
	argv[5] = L"..\\Bin\\input_3x3_0.obu";
	argv[6] = L"..\\Bin\\input_3x3_1.obu";
	argv[7] = L"..\\Bin\\input_3x3_2.obu";
	argv[8] = L"..\\Bin\\input_3x3_3.obu";
	argv[9] = L"..\\Bin\\input_3x3_4.obu";
	argv[10] = L"..\\Bin\\input_3x3_5.obu";
	argv[11] = L"..\\Bin\\input_3x3_6.obu";
	argv[12] = L"..\\Bin\\input_3x3_7.obu";
	argv[13] = L"..\\Bin\\input_3x3_8.obu";
	argc = 13 + 1;
#else
	//Example: "stitcher.exe 2 2 str1.265 str2.265 str3.265 str4.265"
    argv[1] = L"..\\Bin\\outstream_2.265";
    argv[2] = L"3";             //numTileRows
    argv[3] = L"3";             //numTileCols
    argv[4] = L"..\\Bin\\output_000_0.hevc";
    argv[5] = L"..\\Bin\\output_000_1.hevc";
    argv[6] = L"..\\Bin\\output_000_2.hevc";
    argv[7] = L"..\\Bin\\output_000_3.hevc";
    argv[8] = L"..\\Bin\\output_000_4.hevc";
    argv[9] = L"..\\Bin\\output_000_5.hevc";
    argv[10] = L"..\\Bin\\output_000_6.hevc";
    argv[11] = L"..\\Bin\\output_000_7.hevc";
    argv[12] = L"..\\Bin\\output_000_8.hevc";
    argc = 12 + 1;
#endif

    char *cFileNames[MAX_STREAMS + 1];

    uint32_t uiNumStreams;
    uint32_t uiNumTileRows;
    uint32_t uiNumTileCols;
	bool bAnnexBFlag;

    if (argc <= 1)
        showHelp();

    uiNumTileRows = _wtoi(argv[2]);
    uiNumTileCols = _wtoi(argv[3]);
    uiNumStreams = uiNumTileRows * uiNumTileCols;
	bAnnexBFlag = _wtoi(argv[4]);
    // Error handling..
    if (uiNumStreams != argc - 5)
    {
        fprintf(stdout, "Error: the number of tiles (%d) is not equal to the number of input bitstreams (%d)", uiNumStreams, argc);
        goto MAIN_FAIL;
    }
    if (uiNumStreams > MAX_STREAMS - 1)
    {
        fprintf(stdout, "Error: the number of tiles (%d) shall be lower than the number of input bitstreams (%d)", uiNumStreams, MAX_STREAMS);
        goto MAIN_FAIL;
    }

    // open bitstreams
    for (int i = 0; i < (int)uiNumStreams + 1; i++)
    {
        cFileNames[i] = (char *)calloc(MAX_FILENAME_SIZE, sizeof(char));
        UnicodeToMultiByte(i == 0 ? argv[1] : argv[4 + i], cFileNames[i], MAX_FILENAME_SIZE);
    }

    // read bitstreams and merging into single bitstream
    StartBStrStitching(uiNumTileRows, uiNumTileCols, cFileNames, bAnnexBFlag);

MAIN_FAIL:

    for (int i = 0; i < (int)uiNumStreams; i++)
        free(cFileNames[i]);
 
    return 0;
}

void StartBStrStitching(uint32_t uiNumTileRows, uint32_t uiNumTileCols, char **cFileNames, bool bAnnexB)
{
    sThreadArg threadArg = { uiNumTileRows, uiNumTileCols, cFileNames, bAnnexB };

    // stitching thread creation
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &OnBStrStitchProc, (void*)&threadArg, 0, NULL);
    if (!hThread)
    {
        printf("ERROR: Thread creation failure!\n");
        return;
    }

    // stitching...

    WaitForSingleObject(hThread, INFINITE);
}
#if AV1
// Example of bitstream stitching process
uint32_t __stdcall OnBStrStitchProc(void* pThis)
{
    sThreadArg* pThreadArg = (sThreadArg*)pThis;

    uint32_t    uiNumTileRows = pThreadArg->uiNumTileRows;
    uint32_t    uiNumTileCols = pThreadArg->uiNumTileCols;
    uint32_t    uiNumBStreams = pThreadArg->uiNumTileRows * pThreadArg->uiNumTileCols;
	bool        bAnnexBFlag = pThreadArg->bAnnexB;

    CETimer     StitchTimer;
    CETimer     FileReadTimer;
	CETimer		FileWriteTimer;
    LONGLONG    llAccReadTime = 0;
	LONGLONG	llAccWriteTime = 0;

    OBU*  pAV1OBU = new OBU [uiNumBStreams]; // OBU for input bit stream
    CAV1File*   pAV1Files = new CAV1File [uiNumBStreams]; //input bit stream files 
	OBU  AV1OBUOut;  //OBU for output bit stream
	CAV1File   AV1FileOut; //Output bit stream file 

	uint8_t*    pInputBitBuffers[MAX_STREAMS] = { NULL };    // buffers for extracted access units of input bitstream 

    void*       pcBStrStitcherHandle = NULL;      // Handle of stitcher object

    /* Control-C handler */
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        fprintf(stdout, "Unable to register CTRL+C handler: %s\n", strerror(errno));

    fprintf(stdout, "##### Start of stitching thread #####\n\n");

	char **pcInFileName = new char*[uiNumBStreams];
	FILE **pInFile = new FILE*[uiNumBStreams];

    // 1) open input/output bitstream files
    //for (int i = 0; i < (int)uiNumBStreams; i++)
    //{
    //    if (!pAV1Files[i].Open(pThreadArg->cFileNames[i + 1], LargeFile::OM_READONLY))
    //    {
    //        fprintf(stdout, "Failure: cannot open the input file, %s\n", pThreadArg->cFileNames[i + 1]);
    //        goto PROC_FAIL;
    //    }
    //}

	//1-1) OBU file
	for (int i = 0; i < (int)uiNumBStreams; i++)
	{
		if (!pAV1Files[i].OpenOBU(pThreadArg->cFileNames[i + 1]))
		{
			fprintf(stdout, "Failure: cannot open the input OBU file, %s\n", pThreadArg->cFileNames[i + 1]);
			goto PROC_FAIL;
		}
	}

    //if (!AV1FileOut.Open(pThreadArg->cFileNames[0], LargeFile::OM_WRITEONLY))
    //{
    //    fprintf(stdout, "Failure: failed to open bitstream file `%s' for writing\n", pThreadArg->cFileNames[0]);
    //    goto PROC_FAIL;
    //}

    // 2) create input bitstream buffers
	for (int i = 0; i < (int)uiNumBStreams; i++)
	{
		pInputBitBuffers[i] = new uint8_t[MAX_DEC_BUF];
	}

    // 3) create bitstream stitcher class
    if (!Keti_AV1_Stitcher_Create(&pcBStrStitcherHandle, uiNumTileRows, uiNumTileCols, bAnnexBFlag))
    {
        fprintf(stdout, "Failure: cannot create stitcher!!\n");
        goto PROC_FAIL;
    }

    // 4) extract and stitch header information
    StitchTimer.Start();
    uint32_t uiNumFrames = 0;
    uint32_t uiStitchFlags = 0;

	for (int i = 0; i < (int)uiNumBStreams; i++)
	{
		int ret = pAV1Files[i].file_is_obu();

		if (ret = RET_FALSE_AV1)
		{
			fprintf(stdout, "Failure: HEVC bitstream file: reading failure!\n");
			goto PROC_FAIL;
		}
	}

    while (!b_ctrl_c)// && uiNumFrames < 5)
    {
        FileReadTimer.Start();

        // extract access units from bitstreams
        for (int i = 0; i < (int)uiNumBStreams; i++)
        {
			size_t bytes_in_buffer = 0;
			size_t buffer_size = 0;


			//int ret = pAV1Files[i].file_is_obu();
			pAV1Files[i].obudec_read_temporal_unit(&pInputBitBuffers[i], &bytes_in_buffer,&buffer_size);
			int ret = 1; 

            //int ret = pAV1Files[i].ExtractOBU(pInputBitBuffers[i], MAX_DEC_BUF, &pAV1OBU[i]); //각각의 stream에 대한 AU의 정보(AU 시작 위치, 크기, AU 객수) 를 출력
            if (ret == RET_FALSE_AV1)
            {
                fprintf(stdout, "Failure: HEVC bitstream file: reading failure!\n");
                goto PROC_FAIL;
            }
            else if (ret == RET_EOF_AV1)
            {
                //eof
                goto PROC_END;
            }

			pAV1OBU[i].pEachOBU[0] = pInputBitBuffers[i];
			pAV1OBU[i].uiEachOBUSize[0] = buffer_size;
			


            //if (pAV1OBU[i].uiNumOfOBU == 0)
            //{
            //    fprintf(stdout, "Error: Can not find start code!\n");
            //    goto PROC_END;
            //}

            //pAV1OBU[i].pEachOBU[pAV1OBU[i].uiNumOfOBU] = NULL;  // Indicate last NALU boundary
            //pAV1OBU[i].uiEachOBUSize[pAV1OBU[i].uiNumOfOBU] = 0;
        }

		llAccReadTime += FileReadTimer.Stop();

        // stitch all access units into single stream (HevcAuOut updated)
        uiStitchFlags = (uiNumFrames) ? NO_INPUT_FLAGS_AV1 : WRITE_GLB_HDRS_AV1;
		if (Keti_AV1_Stitcher_StitchSingleOBU(pcBStrStitcherHandle, &AV1OBUOut, pAV1OBU, uiStitchFlags))
		{
			//uint32_t uiObuSize = 0;
			//for (uint32_t i = 0; i < HevcAuOut.uiNumOfNALU; i++) {
			//	uiAuSize += HevcAuOut.uiNALUSize[i];
			//}
			//FileWriteTimer.Start();
			//// 5) write memory buffer into file
			//HevcFileOut.Write(HevcAuOut.pNALU[0], uiAuSize);
			//fprintf(stdout, "\r%d frames are completely stitched..\n", ++uiNumFrames);
			//llAccWriteTime += FileWriteTimer.Stop();
		}
		else
		{
			fprintf(stdout, "ERROR: \r%d-th frame stitching failed!\n", uiNumFrames);
		}
    }

PROC_END:

    StitchTimer.Stop();
    LONGLONG stitch_time = StitchTimer.DurationInMilliSeconds();
    LONGLONG read_time = FileReadTimer.calcDurationInMilliSeconds(llAccReadTime);
	LONGLONG write_time = FileWriteTimer.calcDurationInMilliSeconds(llAccWriteTime);
	fprintf(stdout, "\nTotal Frames: %d, Stitching Speed: %.2f (fps)\nTotal Stitching Time: %I64d (msec), File Read Time: %I64d, File Write Time: %I64d\n",
        uiNumFrames, (double)(uiNumFrames * 1000.0 / (stitch_time - read_time - write_time)), stitch_time, read_time, write_time);

    // 6) Destroy
	if (pcBStrStitcherHandle)
	{
		Keti_AV1_Stitcher_Destroy(pcBStrStitcherHandle);
	}

PROC_FAIL:

    for (int i = 0; i < (int)uiNumBStreams; i++)
    {
        if (pInputBitBuffers[i])
        {
            delete [] pInputBitBuffers[i];
			pInputBitBuffers[i] = NULL;
        }
    }

	//if (pAV1Files)
	//{
	//	for (int i = 0; i < (int)uiNumBStreams; i++)
	//		pAV1Files[i].Close();
	//	delete[] pAV1Files;
	//}
 //   AV1FileOut.Close();

	if (pAV1OBU)
	{
		delete[] pAV1OBU;
	}

    if (b_ctrl_c)
        fprintf(stdout, "aborted at input frame %d (by ctrl+c)\n", uiNumFrames);

    fprintf(stdout, "\n#####  End of stitching thread  #####\n");

    return 0;
}
#else
// Example of bitstream stitching process
uint32_t __stdcall OnBStrStitchProc(void* pThis)
{
	sThreadArg* pThreadArg = (sThreadArg*)pThis;

	uint32_t    uiNumTileRows = pThreadArg->uiNumTileRows;
	uint32_t    uiNumTileCols = pThreadArg->uiNumTileCols;
	uint32_t    uiNumBStreams = pThreadArg->uiNumTileRows * pThreadArg->uiNumTileCols;

	CETimer     StitchTimer;
	CETimer     FileReadTimer;
	CETimer		FileWriteTimer;
	LONGLONG    llAccReadTime = 0;
	LONGLONG	llAccWriteTime = 0;

	bool        bAnnexBFlag = true;

	AccessUnit*  pHevcAu = new AccessUnit[uiNumBStreams]; // AU for input bit stream
	CHEVCFile*   pHevcFiles = new CHEVCFile[uiNumBStreams]; //input bit stream files 
	AccessUnit  HevcAuOut;  //AU for output bit stream
	CHEVCFile   HevcFileOut; //Output bit stream file 

	uint8_t*    pInputBitBuffers[MAX_STREAMS] = { NULL };    // buffers for extracted access units of input bitstream 

	void*       pcBStrStitcherHandle = NULL;      // Handle of stitcher object

												  /* Control-C handler */
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
		fprintf(stdout, "Unable to register CTRL+C handler: %s\n", strerror(errno));

	fprintf(stdout, "##### Start of stitching thread #####\n\n");

	// 1) open input/output bitstream files
	for (int i = 0; i < (int)uiNumBStreams; i++)
	{
		if (!pHevcFiles[i].Open(pThreadArg->cFileNames[i + 1], LargeFile::OM_READONLY))
		{
			fprintf(stdout, "Failure: cannot open the input file, %s\n", pThreadArg->cFileNames[i + 1]);
			goto PROC_FAIL;
		}
	}
	if (!HevcFileOut.Open(pThreadArg->cFileNames[0], LargeFile::OM_WRITEONLY))
	{
		fprintf(stdout, "Failure: failed to open bitstream file `%s' for writing\n", pThreadArg->cFileNames[0]);
		goto PROC_FAIL;
	}

	// 2) create input bitstream buffers
	for (int i = 0; i < (int)uiNumBStreams; i++)
	{
		pInputBitBuffers[i] = new uint8_t[MAX_DEC_BUF];
	}

	// 3) create bitstream stitcher class
	if (!Keti_Stitcher_Create(&pcBStrStitcherHandle, uiNumTileRows, uiNumTileCols, bAnnexBFlag))
	{
		fprintf(stdout, "Failure: cannot create stitcher!!\n");
		goto PROC_FAIL;
	}

	// 4) extract and stitch header information
	StitchTimer.Start();
	uint32_t uiNumFrames = 0;
	uint32_t uiStitchFlags = 0;
	while (!b_ctrl_c)// && uiNumFrames < 5)
	{
		FileReadTimer.Start();

		// extract access units from bitstreams
		for (int i = 0; i < (int)uiNumBStreams; i++)
		{
			int ret = pHevcFiles[i].ExtractAU(pInputBitBuffers[i], MAX_DEC_BUF, &pHevcAu[i]); //각각의 stream에 대한 AU의 정보(AU 시작 위치, 크기, AU 객수) 를 출력
			if (ret == RET_FALSE)
			{
				fprintf(stdout, "Failure: HEVC bitstream file: reading failure!\n");
				goto PROC_FAIL;
			}
			else if (ret == RET_EOF)
			{
				//eof
				goto PROC_END;
			}

			if (pHevcAu[i].uiNumOfNALU == 0)
			{
				fprintf(stdout, "Error: Can not find start code!\n");
				goto PROC_END;
			}

			pHevcAu[i].pNALU[pHevcAu[i].uiNumOfNALU] = NULL;  // Indicate last NALU boundary
			pHevcAu[i].uiNALUSize[pHevcAu[i].uiNumOfNALU] = 0;
		}

		llAccReadTime += FileReadTimer.Stop();

		// stitch all access units into single stream (HevcAuOut updated)
		uiStitchFlags = (uiNumFrames) ? NO_INPUT_FLAGS : WRITE_GLB_HDRS;
		if (Keti_Stitcher_StitchSingleAU(pcBStrStitcherHandle, &HevcAuOut, pHevcAu, uiStitchFlags))
		{
			uint32_t uiAuSize = 0;
			for (uint32_t i = 0; i < HevcAuOut.uiNumOfNALU; i++) {
				uiAuSize += HevcAuOut.uiNALUSize[i];
			}
			FileWriteTimer.Start();
			// 5) write memory buffer into file
			HevcFileOut.Write(HevcAuOut.pNALU[0], uiAuSize);
			fprintf(stdout, "\r%d frames are completely stitched..\n", ++uiNumFrames);
			llAccWriteTime += FileWriteTimer.Stop();
		}
		else
		{
			fprintf(stdout, "ERROR: \r%d-th frame stitching failed!\n", uiNumFrames);
		}
	}

PROC_END:

	StitchTimer.Stop();
	LONGLONG stitch_time = StitchTimer.DurationInMilliSeconds();
	LONGLONG read_time = FileReadTimer.calcDurationInMilliSeconds(llAccReadTime);
	LONGLONG write_time = FileWriteTimer.calcDurationInMilliSeconds(llAccWriteTime);
	fprintf(stdout, "\nTotal Frames: %d, Stitching Speed: %.2f (fps)\nTotal Stitching Time: %I64d (msec), File Read Time: %I64d, File Write Time: %I64d\n",
		uiNumFrames, (double)(uiNumFrames * 1000.0 / (stitch_time - read_time - write_time)), stitch_time, read_time, write_time);

	// 6) Destroy
	if (pcBStrStitcherHandle)
	{
		Keti_Stitcher_Destroy(pcBStrStitcherHandle);
	}

PROC_FAIL:

	for (int i = 0; i < (int)uiNumBStreams; i++)
	{
		if (pInputBitBuffers[i])
		{
			delete[] pInputBitBuffers[i];
			pInputBitBuffers[i] = NULL;
		}
	}

	if (pHevcFiles)
	{
		for (int i = 0; i < (int)uiNumBStreams; i++)
			pHevcFiles[i].Close();
		delete[] pHevcFiles;
	}
	HevcFileOut.Close();

	if (pHevcAu)
	{
		delete[] pHevcAu;
	}

	if (b_ctrl_c)
		fprintf(stdout, "aborted at input frame %d (by ctrl+c)\n", uiNumFrames);

	fprintf(stdout, "\n#####  End of stitching thread  #####\n");

	return 0;
}
#endif

//bool xWriteIntoBuffer(uint8_t* pOutBuf, uint64_t& ruiOutBufPos, AccessUnit* pOutAUs)
//{
//    if (!pOutAUs->uiNumOfNALU)
//        return false;
//
//    uint32_t uiSrcDataSize = 0;
//    uint8_t* pBuffer = pOutBuf + ruiOutBufPos;
//    for (uint32_t i = 0; i < pOutAUs->uiNumOfNALU; i++)
//    {
//        memcpy(pBuffer, pOutAUs->pNALU[i], pOutAUs->uiNALUSize[i]);
//        pBuffer += pOutAUs->uiNALUSize[i];
//        uiSrcDataSize += pOutAUs->uiNALUSize[i];
//    }
//    ruiOutBufPos += uiSrcDataSize;
//
//    return true;
//}
