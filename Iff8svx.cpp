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


//////////////////// public methods

CIff8svx::CIff8svx(void)
	: CIffContainer()
	, m_File()
	, m_pHead(nullptr)
{
}

CIff8svx::~CIff8svx(void)
{
	if (m_pHead != nullptr)
	{
		delete m_pHead;
		m_pHead = nullptr;
	}
	m_File.Destroy();
}

bool CIff8svx::ParseFile(LPCTSTR szPathName)
{
	if (m_File.Create(szPathName) == false)
	{
		return false;
	}

	m_pHead = ParseIffFile(m_File);
	if (m_pHead == nullptr)
	{
		return false;
	}

	// we expect 8SVX-type IFF-file
	if (m_pHead->m_iFileID != MakeTag("8SVX"))
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
		uint8_t *pData = CIffContainer::GetViewByOffset(pChunk->m_iOffset, m_File);

		//int64_t ickEnd = (pChunk->m_iOffset + pChunk->m_iChunkSize);
		//int64_t iChOffset = pChunk->m_iOffset;

		// suitable handling for chunk data..
		if (pChunk->m_iChunkID == MakeTag("VHDR"))
		{
			//uint32_t iSamplesHigh1 = Swap4(GetValueAtOffset(iChOffset));
			//iChOffset += sizeof(uint32_t):
			Voice8Header *pVoxHdr = (Voice8Header*)pData;
		}
		else if (pChunk->m_iChunkID == MakeTag("NAME"))
		{
			// string-data (CHAR[])
		}
		else if (pChunk->m_iChunkID == MakeTag("AUTH"))
		{
			// string-data (CHAR[])
		}
		else if (pChunk->m_iChunkID == MakeTag("ANNO"))
		{
			// string-data (CHAR[])
		}
		else if (pChunk->m_iChunkID == MakeTag("(c) "))
		{
			// string-data (CHAR[])
		}
		else if (pChunk->m_iChunkID == MakeTag("ATAK"))
		{
		}
		else if (pChunk->m_iChunkID == MakeTag("RLSE"))
		{
		}
		else if (pChunk->m_iChunkID == MakeTag("BODY"))
		{
			// just signed bytes as data
			int8_t *pData = new int8_t[pChunk->m_iChunkSize];
		}

		pChunk = pChunk->m_pNext;
	}

	return true;
}
