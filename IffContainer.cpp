/////////////////////////////////////////////////////////
//
// CIffContainer : generic IFF-format parser,
// detailed parsing must be per file-type (format)
// since IFF is more generic.
//
// (c) Ilkka Prusi, 2011
//


//#include "stdafx.h"
#include "IffContainer.h"


uint16_t CIffContainer::Swap2(const uint16_t val) const
{
   return (((val >> 8)) | (val << 8));
}

uint32_t CIffContainer::Swap4(const uint32_t val) const
{
	// swap bytes
	return (
			((val & 0x000000FF) << 24) + ((val & 0x0000FF00) <<8) |
			((val & 0x00FF0000) >> 8) + ((val & 0xFF000000) >>24)
			);
}

/* 32-bit format but some may use 64-bit values also?
uint64_t CIffContainer::Swap8(const uint64_t val) const
{
}
*/

float CIffContainer::SwapF(const float fval) const
{
	// must have value where we can take address (not from function parameter),
	// cast via "bit-array" to another type:
	// avoid implicit "float-int" conversion to avoid rounding errors.
	//
	float fTemp = fval;
	uint32_t tmp = Swap4((*((uint32_t*)(&fTemp))));
	fTemp = (*((float*)(&tmp)));
	return fTemp;
}

/* 32-bit format but some may use 64-bit values also?
double CIffContainer::SwapD(const double dval) const
{
	// similar to byteswap on float,
	// avoid int<->float conversion/rounding errors during byteswap
	//
	double dTemp = dval;
	uint64_t tmp = Swap8((*((uint64_t*)(&dTemp))));
	dTemp = (*((double*)(&tmp)));
	return dTemp;
}
*/

// combine individual chars to tag in 32-bit int
uint32_t CIffContainer::MakeTag(const char *buf) const
{
	// note: little-endian CPU with big-endian data
	uint32_t tmp = 0;
	tmp |= (((uint32_t)(buf[3])) << 24);
	tmp |= (((uint32_t)(buf[2])) << 16);
	tmp |= (((uint32_t)(buf[1])) << 8);
	tmp |= ((uint32_t)(buf[0]));
	return tmp;

	/*
	// big-endian CPU with big-endian data
	uint32_t tmp = 0;
	tmp |= (((uint32_t)(buf[0])) << 24);
	tmp |= (((uint32_t)(buf[1])) << 16);
	tmp |= (((uint32_t)(buf[2])) << 8);
	tmp |= ((uint32_t)(buf[3]));
	return tmp;
	*/
}

uint32_t CIffContainer::GetValueAtOffset(const int64_t iOffset, CMemoryMappedFile &pFile) const
{
	// byte-pointer to given offset, get value as 4-byte int
	uint8_t *pData = (uint8_t*)pFile.GetView();
	pData = pData + iOffset;
	return (*((uint32_t*)pData));
}

uint8_t *CIffContainer::GetViewByOffset(const int64_t iOffset, CMemoryMappedFile &pFile) const
{
	uint8_t *pData = (uint8_t*)pFile.GetView();
	return (pData + iOffset);
}

CIffHeader *CIffContainer::ReadHeader(int64_t &iOffset, CMemoryMappedFile &pFile) const
{
	// at least header must exist in file
	if (iOffset+8 > pFile.GetSize())
	{
		return nullptr;
	}

	// in case we are reading composite-FORM -> offset view
	uint32_t *pData = (uint32_t*)GetViewByOffset(iOffset, pFile);
	CIffHeader *pHeader = new CIffHeader();
	pHeader->m_iFileID = pData[0]; // generic type, "FORM" usually

	// keep values from header and byteswap (big->little),
	// size before ID in header
	pHeader->m_iDataSize = Swap4(pData[1]); // datasize according to header

	iOffset += 8; // after header
	if (! (iOffset+4 > pFile.GetSize()))
	{
		pHeader->m_iTypeID = pData[2]; // actual file type (e.g. ILBM);
		iOffset += 4;
	}
	
	// position to where "header-data" (chunks) are..
	pHeader->m_iOffset = iOffset;
	return pHeader;
}

CIffChunk *CIffContainer::ReadNextChunk(int64_t &iOffset, CMemoryMappedFile &pFile) const
{
	// no more chunks
	if (iOffset+8 > pFile.GetSize())
	{
		return nullptr;
	}

	CIffChunk *pCurrent = new CIffChunk();

	// ID followed by size
	pCurrent->m_iChunkID = GetValueAtOffset(iOffset, pFile);
	pCurrent->m_iChunkSize = Swap4(GetValueAtOffset(iOffset+4, pFile));

	// keep chunk "raw" data position in file
	pCurrent->m_iOffset = iOffset +8;

	// offset to next chunk start
	//
	// note: when chunk has and odd-size of data
	// we must offset by 1 for padding since each chunk
	// should start at even-sized offset.
	
	iOffset += (pCurrent->m_iChunkSize +8);
	if ((pCurrent->m_iChunkSize % 2) != 0)
	{
		// odd -> even
		iOffset += 1;
	}
	return pCurrent;
}

void CIffContainer::ReadChunks(int64_t &iOffset, CIffHeader *pHeader, CMemoryMappedFile &pFile)
{
	CIffChunk *pPrev = pHeader->m_pFirst;

	// verify we end also:
	// in case of padding we might have infinite loop
	// (smaller by one)
	//
	while (iOffset+8 < pFile.GetSize())
	{
		CIffChunk *pNext = ReadNextChunk(iOffset, pFile);
		if (pNext != nullptr)
		{
			pNext->m_pPrevious = pPrev;
			if (pPrev != nullptr)
			{
				// not first -> link to new
				pPrev->m_pNext = pNext;
			}
			else
			{
				// first -> keep on header
				pHeader->m_pFirst = pNext;
			}
			pPrev = pNext;
		}

		// call virtual method which user can implement:
		// allows single-pass processing of file-contents
		OnChunk(pPrev, pFile);
		
		// TODO: sub-chunks of node?
		// (these are file-type and node specific if any..)
		//CreateSubChunkNode(pPrev, iOffset, pFile);
		
		// composite FORM?
		if (pPrev->m_iChunkID == MakeTag("FORM"))
		{
			// read new composite
			CIffHeader *pNewHead = ReadHeader(iOffset, pFile);
			if (pNewHead != nullptr)
			{
				/* // need to rethink..
				if (pHeader->m_pParent != nullptr)
				{
					pNewHead->m_pParent = pHeader->m_pParent;
				}
				else
				{
					pNewHead->m_pParent = pHeader;
				}
				*/
				pHeader->AddComposite(pNewHead);
				
				// read chunks of found composite
				ReadChunks(iOffset, pNewHead, pFile);
			}
		}
	}
}


/////////////// public methods

CIffContainer::CIffContainer(void)
    : m_pHeader(nullptr)
{
}

CIffContainer::~CIffContainer(void)
{
	if (m_pHeader != nullptr)
	{
		delete m_pHeader;
	}
}

CIffHeader *CIffContainer::ParseIffFile(CMemoryMappedFile &pFile)
{
	if (pFile.IsCreated() == false)
	{
		return nullptr;
	}

	uint32_t *pData = (uint32_t*)pFile.GetView();

	// must have proper IFF-identifier at start
	//
	// note: also may have LIST or CAT with FORM sub-chunk
	// (e.g. anim-file may include both pics and sound)
	//
	if (pData[0] != MakeTag("FORM")
	    && pData[0] != MakeTag("LIST")
	    && pData[0] != MakeTag("CAT ")) 
	{
		// nothing to do, unsupported file
		return nullptr;
	}

	int64_t iOffset = 0;

	// start of file
	m_pHeader = ReadHeader(iOffset, pFile);
	if (m_pHeader == nullptr)
	{
		// failure reading?
		return nullptr;
	}
	
	// inherited can re-implement to allow only some types
	// such as only 8SVX or ILBM etc.
	IsSupportedType(m_pHeader);
	
	// read chunks, recursion when necessary
	ReadChunks(iOffset, m_pHeader, pFile);

	return m_pHeader;
}

