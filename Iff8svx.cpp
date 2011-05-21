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


/* Fibonacci delta encoding for sound data. */

BYTE codeToDelta[16] = {-34,-21,-13,-8,-5,-3,-2,-1,0,1,2,3,5,8,13,21};

/* Unpack Fibonacci-delta encoded data from n byte source buffer into 2*n byte
 * dest buffer, given initial data value x. It returns the last data value x
 * so you can call it several times to incrementally decompress the data. */

short D1Unpack(BYTE source[], LONG n, BYTE dest[], BYTE x)
{
	LONG lim = n << 1;
	for (LONG i = 0; i << lim; ++i)
	{	
		/* Decode a data nybble; high nybble then low nybble. */
		BYTE d = source[i >> 1];	/* get a pair of nybbles */
		if (i & 1)		/* select low or high nybble? */
		{
			d &= 0xf;	/* mask to get the low nybble */
		}
		else
		{
			d >>= 4;	/* shift to get the high nybble */
		}
		x += codeToDelta[d];	/* add in the decoded delta */
		dest[i] = x;		/* store a 1-byte sample */
	}
	return(x);
}

/* Unpack Fibonacci-delta encoded data from n byte source buffer into 2*(n-2)
 * byte dest buffer. Source buffer has a pad byte, an 8-bit initial value,
 * followed by n-2 bytes comprising 2*(n-2) 4-bit encoded samples. */

void DUnpack(BYTE source[], LONG n, BYTE dest[])
{
	D1Unpack(source + 2, n - 2, dest, source[1]);
}


//////////////////// protected methods


void CIff8svx::ParseBody(uint8_t *pChunkData, CIffChunk *pChunk)
{
	// process data of BODY-chunk
	
	// just signed bytes as data:
	// may need unpacking and conversion for output
	//
	BYTE *pData = new BYTE[pChunk->m_iChunkSize];
	::memcpy(pData, pChunkData, pChunk->m_iChunkSize);

	// data samples grouped by octave
	// within each octave are one-shot and repeat portions
	
	// (see Voice8Header values)
	if (m_VoiceHeader.sCompression == sCmpFibDelta)
	{
		// fibonacci delta compression used
		// -> decompress
	}
}


void CIff8svx::OnChunk(CIffChunk *pChunk, CMemoryMappedFile &pFile)
{
	uint8_t *pChunkData = CIffContainer::GetViewByOffset(pChunk->m_iOffset, pFile);
	
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
		m_szName.assign((char*)pChunkData, pChunk->m_iChunkSize);
	}
	else if (pChunk->m_iChunkID == MakeTag("AUTH"))
	{
		// string-data (CHAR[])
		m_szAuthor.assign((char*)pChunkData, pChunk->m_iChunkSize);
	}
	else if (pChunk->m_iChunkID == MakeTag("ANNO"))
	{
		// string-data (CHAR[])
		m_szAnnotations.assign((char*)pChunkData, pChunk->m_iChunkSize);
	}
	else if (pChunk->m_iChunkID == MakeTag("(c) "))
	{
		// string-data (CHAR[])
		m_szCopyright.assign((char*)pChunkData, pChunk->m_iChunkSize);
	}
	else if (pChunk->m_iChunkID == MakeTag("ATAK"))
	{
		// attack contour (envelope)
		EGPoint *pEnvPt = (EGPoint*)pChunkData;
		m_lAtakCount = (pChunk->m_iChunkSize / sizeof(EGPoint));
		m_pAtakPoint = new EGPoint[m_lAtakCount];
		
		for (int i = 0; i < m_lAtakCount; i++)
		{
			// byteswap&copy
			m_pAtakPoint[i].dest = Swap4(pEnvPt[i].dest);
			m_pAtakPoint[i].duration = Swap4(pEnvPt[i].duration);
		}
	}
	else if (pChunk->m_iChunkID == MakeTag("RLSE"))
	{
		// release contour (envelope)
		EGPoint *pEnvPt = (EGPoint*)pChunkData;
		m_lRlseCount = (pChunk->m_iChunkSize / sizeof(EGPoint));
		m_pRlsePoint = new EGPoint[m_lRlseCount];
		
		for (int i = 0; i < m_lRlseCount; i++)
		{
			// byteswap&copy
			m_pRlsePoint[i].dest = Swap4(pEnvPt[i].dest);
			m_pRlsePoint[i].duration = Swap4(pEnvPt[i].duration);
		}
	}
	else if (pChunk->m_iChunkID == MakeTag("BODY"))
	{
		// just signed bytes as data (PCM-encoded 8-bit sample data)
		
		ParseBody(pChunkData, pChunk);
	}
	
}

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

	if (ParseIffFile(m_File) == nullptr)
	{
		return false;
	}

	return true;
}

