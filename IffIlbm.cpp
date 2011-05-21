/////////////////////////////////////////////////////////
//
// CIffIlbm : IFF-ILBM image format parser
//
// (c) Ilkka Prusi, 2011
//
// See format specs from: 
// http://www.fine-view.com/jp/labs/doc/ilbm.txt
//

//#include "stdafx.h"
#include "IffIlbm.h"


// TODO: subclass from chunk and do processing there?
//
bool CIffIlbm::ParseBitmapHeader(uint8_t *pChunkData, CIffChunk *pChunk)
{
	// BMHD bitmap header chunk:
	// should be first in file (according to spec)
	// and needed by later processing of data

	// make byteswapping where necessary to a copy (keep original)
	//BitMapHeader SwappedHd;

	BitMapHeader *pBmHd = (BitMapHeader*)pChunkData;
	m_BmHeader.w = Swap2(pBmHd->w);
	m_BmHeader.h = Swap2(pBmHd->h);
	m_BmHeader.x = Swap2(pBmHd->x);
	m_BmHeader.y = Swap2(pBmHd->y);
	m_BmHeader.nPlanes = pBmHd->nPlanes;
	m_BmHeader.masking = pBmHd->masking;
	m_BmHeader.compression = pBmHd->compression;
	m_BmHeader.pad1 = pBmHd->pad1; // unused
	m_BmHeader.transparentColor = Swap2(pBmHd->transparentColor);
	m_BmHeader.xAspect = pBmHd->xAspect;
	m_BmHeader.yAspect = pBmHd->yAspect;
	m_BmHeader.pageWidth = Swap2(pBmHd->pageWidth);
	m_BmHeader.pageHeight = Swap2(pBmHd->pageHeight);

	return true;
}

// TODO: subclass from chunk and do processing there?
//
void CIffIlbm::ParseBody(uint8_t *pChunkData, CIffChunk *pChunk)
{
	//
	// The pixel values in a BODY can be indexes into the palette contained in a CMAP chunk, 
	// or they can be literal RGB values. 
	// If there is no CMAP and if BMHD.BitPlanes is 24, 
	// the ILBM contains a 24-bit image, 
	// and the BODY encodes pixels as literal RGB values.
	//

	int64_t ickEnd = (pChunk->m_iOffset + pChunk->m_iChunkSize);
	int64_t iChOffset = pChunk->m_iOffset;
	while (iChOffset < ickEnd)
	{
		// read each scanline for each plane
		// also handle compression (if any)
		// and masking-scanline (if any).

		// WORDs or BYTEs ?
		// WORDs (bytes??)

		// note: all rows should be same size (and word-aligned?)
		int iRowBytes = ((m_BmHeader.w + 15) >> 4) << 1;

		// for each plane..
		// with or without compression?
		//if (m_BmHeader.compression == cmpNone)

		for (int j = 0; j < m_BmHeader.h; j++)
		{
			for (int i = 0; i < m_BmHeader.nPlanes; i++)
			{
				// read scanline:
				// output-width should be known already (in header)


				// actually, arrays of ColorRegister ?
				// (depends if CMAP exists..)
				UBYTE *pLine = new UBYTE[iRowBytes];
				//size_t nBytes = sizeof(int16_t)*m_BmHeader.w;

				if (m_BmHeader.compression == cmpNone)
				{
					// no compression -> copy as-is
					::memcpy(pLine, pChunkData, iRowBytes);
					iChOffset += iRowBytes;
				}
				else if (m_BmHeader.compression == cmpByteRun1)
				{
					// decompress scanline
					DecompressByteRun1(pChunkData, ickEnd, iChOffset, pLine);
				}
			}

			// extra bitplane per "row" (see height)
			if (m_BmHeader.masking != mskNone)
			{
				// read masking
				/*
				UBYTE *pLine = new UBYTE[iRowBytes];
				::memcpy(pLine, pChunkData, iRowBytes);
				iChOffset += iRowBytes;
				*/
			}
		}
	}

}


void CIffIlbm::DecompressByteRun1(uint8_t *pData, const int64_t ickEnd, int64_t &iChOffset, UBYTE *pLine)
{
	//for (UWORD i = 0; i < m_BmHeader.w; i++)

	UWORD i = 0;
	while (i < m_BmHeader.w && iChOffset < ickEnd)
	{
		int8_t bMark = (int8_t)pData[iChOffset]; // compression flag
		uint8_t bNext = pData[iChOffset+1]; // byte to replicate or not

		if (bMark >= 0)
		{
			// copy next (bMark +1) bytes as-is
			int j = 0;
			while (j < bMark)
			{
				pLine[i] = pData[iChOffset+1];
				iChOffset++;
				i++;
				j++;
			}
		}
		else if (bMark < 0 && bMark >= -127)
		{
			// replicate bNext (-bMark +1) times

			int j = 0;
			while (j < (bMark*-1))
			{
				pLine[i] = bNext;
				//iChOffset++; // just replicate
				i++;
				j++;
			}

		}
		else if (bMark == -128)
		{
			// noop
			iChOffset++;
		}
	}
}

void CIffIlbm::OnChunk(CIffChunk *pChunk, CMemoryMappedFile &pFile)
{
	// "raw" data of the chunk,
	// locate by offset
	uint8_t *pChunkData = CIffContainer::GetViewByOffset(pChunk->m_iOffset, pFile);

	// suitable handling for chunk data..
	if (pChunk->m_iChunkID == MakeTag("BMHD"))
	{
		// BMHD bitmap header chunk:
		// should be first in file (according to spec)
		// and needed by later processing of data
		ParseBitmapHeader(pChunkData, pChunk);
	}
	else if (pChunk->m_iChunkID == MakeTag("CMAP"))
	{
		// CMAP color map chunk
		int iCount = (pChunk->m_iChunkSize/sizeof(ColorRegister));
		m_pCmap = new ColorRegister[iCount];

		// bytes only, copy as-is: no need for byteswap
		::memcpy(m_pCmap, pChunkData, iCount*sizeof(ColorRegister));
	}
	else if (pChunk->m_iChunkID == MakeTag("GRAB"))
	{
		// GRAB "hotspot" position (optional)
		// (e.g. brush center)
		Point2D *pPt = (Point2D*)pChunkData;
		m_Pt2d.x = Swap2(pPt->x);
		m_Pt2d.y = Swap2(pPt->y);
	}
	else if (pChunk->m_iChunkID == MakeTag("DEST"))
	{
		// DEST (optional)
		DestMerge *pDst = (DestMerge*)pChunkData;
		m_DestMerge.depth = pDst->depth;
		m_DestMerge.pad1 = pDst->pad1; // padding, not necessary
		m_DestMerge.planePick = Swap2(pDst->planePick);
		m_DestMerge.planeOnOff = Swap2(pDst->planeOnOff);
		m_DestMerge.planeMask = Swap2(pDst->planeMask);
	}
	else if (pChunk->m_iChunkID == MakeTag("CRNG"))
	{
		// CRNG (optional), "nonstandard"
		// used by e.g. EA Deluxe Paint
		CRange *pRange = (CRange*)pChunkData;
		m_Range.pad1 = Swap2(pRange->pad1); // padding, not necessary
		m_Range.rate = Swap2(pRange->rate);
		m_Range.active = Swap2(pRange->active);
		m_Range.low = pRange->low;
		m_Range.high = pRange->high;
	}
	else if (pChunk->m_iChunkID == MakeTag("CCRT"))
	{
		// CCRT (optional)
		// (Color Cycling Range and Timing)
		CycleInfo *pCcrt = (CycleInfo*)pChunkData;
		m_CycleInfo.direction = Swap2(pCcrt->direction);
		m_CycleInfo.start = pCcrt->start;
		m_CycleInfo.end = pCcrt->end;
		m_CycleInfo.seconds = Swap4(pCcrt->seconds);
		m_CycleInfo.microseconds = Swap4(pCcrt->microseconds);
		m_CycleInfo.pad = Swap2(pCcrt->pad); // padding, not necessary
	}
	else if (pChunk->m_iChunkID == MakeTag("SPRT"))
	{
		// SPRT (optional),
		// when image intended as a sprite:
		// "Z"-plane order of sprite (0 foremost)
		m_SpriteOrder = Swap2((*(SpritePrecedence*)pChunkData));
	}
	else if (pChunk->m_iChunkID == MakeTag("CAMG"))
	{
		// CAMG (optional),
		// Amiga "viewport mode"
		// e.g. "dual playfield", "hold and modify"
		m_lViewPortMode = Swap4((*((LONG*)pChunkData)));
	}
	else if (pChunk->m_iChunkID == MakeTag("BODY"))
	{
		// BODY raster body chunk
		// content is concatenation of scan lines

		ParseBody(pChunkData, pChunk);
	}
	
}


//////////////////// public methods

CIffIlbm::CIffIlbm(void)
	: CIffContainer()
	, m_File()
	, m_pCmap(nullptr)
{
}

CIffIlbm::~CIffIlbm(void)
{
	if (m_pCmap != nullptr)
	{
		delete m_pCmap;
		m_pCmap = nullptr;
	}
	m_File.Destroy();
}

bool CIffIlbm::ParseFile(LPCTSTR szPathName)
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

