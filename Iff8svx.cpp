/////////////////////////////////////////////////////////
//
// CIff8svx : IFF-8svx audio format parser
//
// (c) Ilkka Prusi, 2011
//
// See format specification at:
// http://amigan.1emu.net/reg/8SVX.txt
//


//#include "stdafx.h"
#include "Iff8svx.h"

#include <string>

//////////////////// public methods

CIff8svx::CIff8svx(void)
	: CIffContainer()
	, m_File()
{
}

CIff8svx::~CIff8svx(void)
{
	m_File.Destroy();
}

bool CIff8svx::ParseFile(LPCTSTR szPathName)
{
	if (m_File.Create(szPathName) == false)
	{
		return false;
	}

	ParseIffFile(m_File);
	if (m_pHeader == nullptr)
	{
		return false;
	}

	return ParseChunks();
}

bool CIff8svx::ParseChunks()
{
	// we expect 8SVX-type IFF-file
	if (m_pHeader->m_iTypeID != MakeTag("8SVX"))
	{
		return false;
	}

	// TODO: make virtual methods in base
	// so that processing can be done on first-pass
	// through file instead of second here?

	CIffChunk *pChunk = m_pHead->m_pFirst;
	while (pChunk != nullptr)
	{
		// "raw" data of the chunk,
		// locate by offset
		uint8_t *pChunkData = CIffContainer::GetViewByOffset(pChunk->m_iOffset, m_File);

		//int64_t ickEnd = (pChunk->m_iOffset + pChunk->m_iChunkSize);
		//int64_t iChOffset = pChunk->m_iOffset;

		// suitable handling for chunk data..
		if (pChunk->m_iChunkID == MakeTag("VHDR"))
		{
			Voice8Header *pVoxHdr = (Voice8Header*)pChunkData;
			m_VoiceHeader.oneShotHiSamples = Swap4(pVoxHdr->oneShotHiSamples);
			m_VoiceHeader.repeatHiSamples = Swap4(pVoxHdr->repeatHiSamples);
			m_VoiceHeader.samplesPerHiCycle = Swap4(pVoxHdr->repeatHiSamples);
			m_VoiceHeader.samplesPerSec = Swap2(pVoxHdr->samplesPerSec);
			m_VoiceHeader.ctOctave = pVoxHdr->ctOctave;
			m_VoiceHeader.sCompression = pVoxHdr->sCompression;
			m_VoiceHeader.volume = Swap4(pVoxHdr->volume);
		}
		else if (pChunk->m_iChunkID == MakeTag("NAME"))
		{
			// string-data (CHAR[])
			//char *pData = new char[pChunk->m_iChunkSize +1];
			std::string szData;
			szData.assign((char*)pChunkData, pChunk->m_iChunkSize);
		}
		else if (pChunk->m_iChunkID == MakeTag("AUTH"))
		{
			// string-data (CHAR[])
			std::string szData;
			szData.assign((char*)pChunkData, pChunk->m_iChunkSize);
		}
		else if (pChunk->m_iChunkID == MakeTag("ANNO"))
		{
			// string-data (CHAR[])
			std::string szData;
			szData.assign((char*)pChunkData, pChunk->m_iChunkSize);
		}
		else if (pChunk->m_iChunkID == MakeTag("(c) "))
		{
			// string-data (CHAR[])
			std::string szData;
			szData.assign((char*)pChunkData, pChunk->m_iChunkSize);
		}
		else if (pChunk->m_iChunkID == MakeTag("ATAK"))
		{
			// attack contour
			EGPoint *pEnvPt = (EGPoint*)pChunkData;
			int iCount = (pChunk->m_iChunkSize / sizeof(EGPoint));
			EGPoint *pAtak = new EGPoint[iCount];
			
			// byteswap&copy
			for (int i = 0; i < iCount; i++)
			{
				pRlse[i].dest = Swap2(pEnvPt[i].dest);
				pRlse[i].duration = Swap4(pEnvPt[i].duration);
			}
		}
		else if (pChunk->m_iChunkID == MakeTag("RLSE"))
		{
			// release contour
			EGPoint *pEnvPt = (EGPoint*)pChunkData;
			int iCount = (pChunk->m_iChunkSize / sizeof(EGPoint));
			EGPoint *pRlse = new EGPoint[iCount];
			
			// byteswap&copy
			for (int i = 0; i < iCount; i++)
			{
				pRlse[i].dest = Swap2(pEnvPt[i].dest);
				pRlse[i].duration = Swap4(pEnvPt[i].duration);
			}
		}
		else if (pChunk->m_iChunkID == MakeTag("BODY"))
		{
			// just signed bytes as data (PCM-encoded 8-bit sample data)
			BYTE *pData = new BYTE[pChunk->m_iChunkSize];
			::memcpy(pData, pChunkData, pChunk->m_iChunkSize);
			
			// data samples grouped by octave
			// within each octave are one-shot and repeat portions
		}

		pChunk = pChunk->m_pNext;
	}

	return true;
}
