/////////////////////////////////////////////////////////
//
// CIff8svx : IFF-8svx audio format parser
// (8-bit Sampled VoX, or Voice)
//
// (c) Ilkka Prusi, 2011
//
// See format specification at:
// http://amigan.1emu.net/reg/8SVX.txt
//

#ifndef _IFF8SVX_H_
#define _IFF8SVX_H_

#include <stdint.h>

#include "MemoryMappedFile.h"
#include "IffContainer.h"

// std::string, for keeping sample-info
#include <string>


// support for old-style decl
//
//typedef int8_t BYTE;
typedef uint8_t UBYTE;
//typedef int16_t WORD;
typedef uint16_t UWORD;
//typedef int32_t LONG;
//typedef uint32_t ULONG;


/* A fixed-point value, 16 bits to the left of the point and 16 
	* to the right. A Fixed is a number of 216ths, i.e. 65536ths.	*/
typedef LONG Fixed;	

#define Unity 0x10000L	/* Unity = Fixed 1.0 = maximum volume	*/

/* sCompression: Choice of compression algorithm applied to the samples. */ 

#define sCmpNone       0	/* not compressed	*/
#define sCmpFibDelta   1	/* Fibonacci-delta encoding (Appendix C)  */


// from format specifications,
// note one-byte alignment in struct

#pragma pack(push, 1)

/* Can be more kinds in the future.	*/

typedef struct 
{
	ULONG oneShotHiSamples;	    /* # samples in the high octave 1-shot part */
    ULONG repeatHiSamples;	    /* # samples in the high octave repeat part */
    ULONG samplesPerHiCycle;	/* # samples/cycle in high octave, else 0   */
	UWORD samplesPerSec;	    /* data sampling rate	*/
	UBYTE ctOctave;		/* # octaves of waveforms	*/
    UBYTE sCompression;		/* data compression technique used	*/
	Fixed volume;		    /* playback volume from 0 to Unity (full 
				 * volume). Map this value into the output 
				 * hardware's dynamic range.	*/
} Voice8Header;


/* ATAK and RLSE chunks contain an EGPoint[], piecewise-linear envelope.*/ 
/* The envelope defines a function of time returning Fixed values. It's
 * used to scale the nominal volume specified in the Voice8Header. */
typedef struct 
{
	UWORD duration;	/* segment duration in milliseconds, > 0	*/
	Fixed dest;	/* destination volume factor	*/
} EGPoint;

#pragma pack(pop)


class CIff8svx : public CIffContainer
{
private:
	CMemoryMappedFile m_File;
	//CIffHeader *m_pHead; // inherited now

protected:

	Voice8Header m_VoiceHeader;
	
	std::string m_szName; // NAME
	std::string m_szAuthor; // AUTH
	std::string m_szAnnotation; // ANNO
	std::string m_szCopyright; // (c)
	
	void ParseBody(uint8_t *pChunkData, CIffChunk *pChunk);

	virtual void OnChunk(CIffChunk *pChunk, CMemoryMappedFile &pFile);
	
	virtual bool IsSupportedType(CIffHeader *pHeader)
	{
		if (pHeader->m_iTypeID == MakeTag("8SVX"))
		{
			return true;
		}
		return false;
	}
	
public:
	CIff8svx(void);
	virtual ~CIff8svx(void);

	bool ParseFile(LPCTSTR szPathName);
	bool ParseChunks();
};

#endif // ifndef _IFF8SVX_H_

