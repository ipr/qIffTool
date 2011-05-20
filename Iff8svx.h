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

// support for old-style decl
//
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

typedef struct {
	ULONG oneShotHiSamples,	/* # samples in the high octave 1-shot part */
        repeatHiSamples,	/* # samples in the high octave repeat part */
        samplesPerHiCycle;	/* # samples/cycle in high octave, else 0   */
	UWORD samplesPerSec;	/* data sampling rate	*/
	UBYTE ctOctave,		/* # octaves of waveforms	*/
      	sCompression;		/* data compression technique used	*/
	Fixed volume;		/* playback volume from 0 to Unity (full 
				 * volume). Map this value into the output 
				 * hardware's dynamic range.	*/
	} Voice8Header;


/* ATAK and RLSE chunks contain an EGPoint[], piecewise-linear envelope.*/ 
/* The envelope defines a function of time returning Fixed values. It's
 * used to scale the nominal volume specified in the Voice8Header. */
typedef struct {
	UWORD duration;	/* segment duration in milliseconds, > 0	*/
	Fixed dest;	/* destination volume factor	*/
	} EGPoint;

#pragma pack(pop)


class CIff8svx : public CIffContainer
{
private:
	CMemoryMappedFile m_File;
	CIffHeader *m_pHead;

protected:


public:
	CIff8svx(void);
	virtual ~CIff8svx(void);

	bool ParseFile(LPCTSTR szPathName);
	bool ParseChunks();
};

#endif // ifndef _IFF8SVX_H_

