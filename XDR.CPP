/*************************************************************************
*
* File Name	 : xdr.cpp
*
* Description: OS/2 XDR library
*
* Exports 	 : xdrstdio_create()   	xdrmem_create()		xdr_destroy()
*			   xdr_free()			xdrmem_base()		xdr_getpos()
*			   xdr_setpos()			xdr_char()			xdr_short()
*			   xdr_u_short()		xdr_int()			xdr_u_int()
*			   xdr_long()			xdr_u_long()		xdr_void()
*			   xdr_enum()			xdr_string()		xdr_opaque()
*			   xdr_bytes()			xdr_vector()		xdr_array()
*			   xdr_union()
*
* Imports	 : memcpy <mem.h>		malloc <stdlib.h>	free <stdlib.h>
*			   realloc <stdlib.h>	fseek <stdio.h>		fread <stdio.h>
*			   fwrite <stdio.h>     ftell <stdio.h>		strlen.h <string.h>
*
* copyright (c) J”rg Caumanns, 1993 [caumanns@cs.tu-berlin.de]
*
*      DISCLAIMER OF WARRANTIES.
*      The code is provided "AS IS", without warranty of any kind.
*
*************************************************************************/
#include <os2def.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <stdlib.h>
#include <xdr.h>

/*
* constants
*/
#define BYTES_PER_XDR_UNIT 	4
#define MEMSIZE_BASE		4096	// page size


/***
* XDRR structure:
*
* In order to provide some kind of object-oriented feeling, the XDR
* structure defined in XDR.H is just a dummy for the 'real' XDR
* structure XDRR defined here.
* All constants and datatypes needed to implement XDRR are defined in this
* module, not in XDR.H. You should only use the datatypes and functions
* defined in XDR.H so that any changes in the XDRR structure are invisible
* to your programs.
***/
typedef enum XDR_STREAM	{
			MEMORY,                 // memory stream
			STDIO	};				// I/O stream

typedef struct XDR_STDIO	{
	FILE *pf;						// file pointer
	};

typedef struct XDR_MEM		{
	ULONG ulSize;					// size of memory buffer
	CHAR  *pb;						// address of memory buffer
	BOOL  fAlloc;					// false -> buffer was preallocated
	};

typedef struct XDRR	{
	XDR_STREAM 	stream;
	XDR_OP		op;
	UINT		offset;
	union {
		XDR_STDIO io;
		XDR_MEM   mem;
		} u;
	};

/*
* Makros
*/
#define RNDUP(x)	((((x)+3) >> 2) << 2)	// round to a multiple of 4

#define OP(xdr)		(((XDRR *)(xdr))->op)	// access certain elements of
#define MEM(xdr)	((((XDRR *)(xdr))->u).mem)	// struct XDRR
#define IO(xdr)		((((XDRR *)(xdr))->u).io)

/*
* forward declarations
*/
static BOOL xdr_read(XDR *, CHAR *, ULONG);
static BOOL xdr_write(XDR *, CHAR *, ULONG);
/*************************************************************************
*
* Names : XDR_Read, XDR_Write
*
* Descr.: Read/Write n Bytes from/to a given memory location/file
*
* Impl. : xdr_write reallocates a memory buffer allocated by
*		  'xdrmem_create' if it's not big enough.
*		  This is different from the 'normal' XDR semantics, but I think
*		  it's quite useful.
*
* Param.: XDR  *xdr		- xdr instance handle
*		  CHAR *pBuf	- buffer holding bytes read / to be written
*		  ULONG cBytes	- bytes to read/write
*
* Return: BOOL fSuccess
*
*************************************************************************/
static BOOL xdr_read(XDR *xdr, CHAR *pBuf, ULONG cBytes)	{

	switch(((XDRR *)xdr)->stream)	{
		case MEMORY:
			if((((XDRR *)xdr)->offset + cBytes) > MEM(xdr).ulSize)
				return FALSE;

			memcpy(pBuf, MEM(xdr).pb + ((XDRR *)xdr)->offset, cBytes);
			((XDRR *)xdr)->offset += cBytes;
			return TRUE;

		case STDIO:
			((XDRR *)xdr)->offset += cBytes;
			return (BOOL)fread(pBuf, 1, cBytes, IO(xdr).pf);
		}
	return FALSE;
	}

static BOOL xdr_write(XDR *xdr, CHAR *pBuf, ULONG cBytes)	{

  again:
	switch(((XDRR *)xdr)->stream)	{
		case MEMORY:
			if((((XDRR *)xdr)->offset + cBytes) > MEM(xdr).ulSize)	{
				if(MEM(xdr).fAlloc)	{
					MEM(xdr).ulSize *= 2;
					MEM(xdr).pb = (CHAR *)realloc(MEM(xdr).pb, MEM(xdr).ulSize);
					goto again;
					}
				else
					return FALSE;
				}

			memcpy(MEM(xdr).pb + ((XDRR *)xdr)->offset, pBuf, cBytes);
			((XDRR *)xdr)->offset += cBytes;
			return TRUE;

		case STDIO:
			((XDRR *)xdr)->offset += cBytes;
			return (BOOL)fwrite(pBuf, 1, cBytes, IO(xdr).pf);
		}
	return FALSE;
	}


/*************************************************************************
*
* Name	: xdrstdio_create
*
* Descr.: associate a XDR stream with a file pointer
*
* Impl. : -
*
* Param.: XDR  *xdr		- xdr handle (return)
		  FILE *pf		- File pointer to associate XDR stream with
*		  XDR_OPT option- option (decode, encode, free)
*
* Return: void
*
*************************************************************************/
void xdrstdio_create(XDR *xdr, FILE *pf, XDR_OP option)	{

	((XDRR *)xdr)->stream	= STDIO;
	((XDRR *)xdr)->op		= option;
	((XDRR *)xdr)->offset	= ftell(pf);
	IO(xdr).pf				= pf;
	}


/*************************************************************************
*
* Name	: xdrmem_create
*
* Descr.: associate a XDR stream with a memory buffer
*
* Impl. : If 'pAddr' or 'size' is 0, 'xdrmem_create' allocates a
*		  memory buffer of size MEMSIZE_BASE (4 KB).
*		  If this buffer is not large enough it is enlarged automatically
*		  by this library. If you use this feature call 'xdrmem_base' to
*		  get the buffer's address.
*
* Param.: XDR *xdr		- xdr handle (return)
*		  CHAR *pAddr	- pointer to begin of memory buffer
*		  ULONG size	- size of memory buffer
*		  XDR_OPT option- option (decode, encode, free)
*
* Return: void
*
*************************************************************************/
void xdrmem_create(XDR *xdr, CHAR *pAddr, ULONG size, XDR_OP option)	{

	((XDRR *)xdr)->stream	= MEMORY;
	((XDRR *)xdr)->op 		= option;
	((XDRR *)xdr)->offset	= 0;
	if((!pAddr) || (!size))	{
		MEM(xdr).pb 		= (CHAR *)malloc(MEMSIZE_BASE);
		MEM(xdr).ulSize 	= MEMSIZE_BASE;
		MEM(xdr).fAlloc		= TRUE;
		}
	else	{
		MEM(xdr).pb 		= pAddr;
		MEM(xdr).ulSize 	= size;
		MEM(xdr).fAlloc		= FALSE;
		}
	}


/*************************************************************************
*
* Names	: xdrmem_base
*
* Descr.: Get the current base address
*
* Impl. : -
*
* Param.: XDR *xdr		- xdr handle
*
* Return: CHAR *base	- base address
*
*************************************************************************/
CHAR *xdrmem_base(XDR *xdr)	{

	return MEM(xdr).pb;
	}


/*************************************************************************
*
* Names	: xdr_getpos, xdr_setpos
*
* Descr.: get/set the current read/write offset
*
* Impl. : If 'xdr' points to a standard I/O stream the file pointer
*		  is returned / moved.
*
* Param.: XDR *xdr		- xdr handle
*		  UINT pos		- new offset (xdr_setpos)
*
* Return: a) UINT offset
*		  b) BOOL fSuccess
*
*************************************************************************/
UINT xdr_getpos(XDR *xdr)	{

	return ((XDRR *)xdr)->offset;
	}

BOOL xdr_setpos(XDR *xdr, UINT pos)	{

	((XDRR *)xdr)->offset = pos;

	if(((XDRR *)xdr)->stream == STDIO)
		return (fseek(IO(xdr).pf, pos, 0) == 0)? TRUE : FALSE;

	return TRUE;
	}


/*************************************************************************
*
* Name	: xdr_destroy
*
* Descr.: free all resources allocated by this library
*
* Impl. : Release memory allocated bx 'xdrmem_create'.
*
* Param.: XDR *xdr		- xdr handle
*
* Return: -
*
*************************************************************************/
VOID xdr_destroy(XDR *xdr)	{

	switch(((XDRR *)xdr)->stream)	{
		case MEMORY:
			if(MEM(xdr).fAlloc)
				free(MEM(xdr).pb);
			break;
		case STDIO:
			break;
		}
	}


/*************************************************************************
*
* Name	: (Short, UShort, Int, ....)
*
* Descr.: Read/Write an object of the given type from/to the XDR stream
*
* Impl. : -
*
* Param.: XDR 	 *xdr		- xdr handle
*		  type_t *pObject	- object to be read/written
*
* Return: BOOL fSuccess
*
*************************************************************************/
/**
*
* xdr_char
*
**/
BOOL xdr_char(XDR *xdr, CHAR *pch)	{
	CHAR tmp[BYTES_PER_XDR_UNIT];

	memset(tmp, 0, BYTES_PER_XDR_UNIT);

	switch(OP(xdr))	{
		case XDR_ENCODE:
			tmp[BYTES_PER_XDR_UNIT-1] = *pch;
			return xdr_write(xdr, tmp, BYTES_PER_XDR_UNIT);

		case XDR_DECODE:
			if(!xdr_read(xdr, tmp, BYTES_PER_XDR_UNIT))
				return FALSE;
			*pch = tmp[BYTES_PER_XDR_UNIT-1];
			return TRUE;

		case XDR_FREE:
			return TRUE;
		}
	return FALSE;
	}


/**
*
* xdr_short
*
**/
BOOL xdr_short(XDR *xdr, SHORT *ps)	{
	CHAR tmp[BYTES_PER_XDR_UNIT];

	switch(OP(xdr))	{
		case XDR_ENCODE:
			tmp[BYTES_PER_XDR_UNIT-1] = ((CHAR *)ps)[0];
			tmp[BYTES_PER_XDR_UNIT-2] = ((CHAR *)ps)[1];
			return xdr_write(xdr, tmp, BYTES_PER_XDR_UNIT);

		case XDR_DECODE:
			if(!xdr_read(xdr, tmp, BYTES_PER_XDR_UNIT))
				return FALSE;
			((CHAR *)ps)[0] = tmp[BYTES_PER_XDR_UNIT-1];
			((CHAR *)ps)[1] = tmp[BYTES_PER_XDR_UNIT-2];
			return TRUE;

		case XDR_FREE:
			return TRUE;
		}
	return FALSE;
	}

/**
*
* xdr_u_short
*
**/
BOOL xdr_u_short(XDR *xdr, USHORT *ps)	{

	return xdr_short(xdr, (SHORT *)ps);
	}


/**
*
* xdr_int
*
**/
BOOL xdr_int(XDR *xdr, INT *p)	{
	CHAR tmp[BYTES_PER_XDR_UNIT];

	switch(OP(xdr))	{
		case XDR_ENCODE:
			tmp[BYTES_PER_XDR_UNIT-1] = ((CHAR *)p)[0];
			tmp[BYTES_PER_XDR_UNIT-2] = ((CHAR *)p)[1];
			tmp[BYTES_PER_XDR_UNIT-3] = ((CHAR *)p)[2];
			tmp[BYTES_PER_XDR_UNIT-4] = ((CHAR *)p)[3];
			return xdr_write(xdr, tmp, BYTES_PER_XDR_UNIT);

		case XDR_DECODE:
			if(!xdr_read(xdr, tmp, BYTES_PER_XDR_UNIT))
				return FALSE;
			((CHAR *)p)[0] = tmp[BYTES_PER_XDR_UNIT-1];
			((CHAR *)p)[1] = tmp[BYTES_PER_XDR_UNIT-2];
			((CHAR *)p)[2] = tmp[BYTES_PER_XDR_UNIT-3];
			((CHAR *)p)[3] = tmp[BYTES_PER_XDR_UNIT-4];
			return TRUE;

		case XDR_FREE:
			return TRUE;
		}
	return FALSE;
	}

/**
*
* xdr_u_int
*
**/
BOOL xdr_u_int(XDR *xdr, UINT *p)	{

	return xdr_int(xdr, (INT *)p);
	}

/**
*
* xdr_long
*
**/
BOOL xdr_long(XDR *xdr, LONG *p)	{

	return xdr_int(xdr, (INT *)p);
	}

/**
*
* xdr_u_long
*
**/
BOOL xdr_u_long(XDR *xdr, ULONG *p)	{

	return xdr_int(xdr, (INT *)p);
	}

/**
*
* xdr_void
*
**/
BOOL xdr_void(XDR *xdr, VOID *p)	{

	return TRUE;
	}

/**
*
* xdr_enum
*
**/
BOOL xdr_enum(XDR *xdr, INT *p)	{

	return xdr_int(xdr, p);
	}



/*************************************************************************
*
* Name	: xdr_string
*
* Descr.: Convert a 0-terminated string to XDR and vice versa
*
* Impl. : A string is represented as <UINT length><bytes[x]>, where x
*		  is a multiple of BYTES_PER_XDR_UNIT (4).
*		  If '*ppsz' is set to NULL at decoding, memory is allocated by
*		  this routine.
** Param.: CHAR **ppsz	- pointer to string
*		  UINT sizeMax	- maximum string length
*
* Return: BOOL fSuccess
*
*************************************************************************/
BOOL xdr_string(XDR *xdr, CHAR **ppsz, UINT sizeMax)	{
	UINT len, i, last;
	CHAR tmp[BYTES_PER_XDR_UNIT];

	switch(OP(xdr))	{
		case XDR_ENCODE:
			if((!*ppsz) || ((len = strlen(*ppsz)) > sizeMax))
				return FALSE;

			xdr_u_int(xdr, &len); 	// write length

			last = (len >> 2) << 2; // write full units
			if(!xdr_write(xdr, *ppsz, last))
				return FALSE;

			if(last != len)	{		// fill last unit with zeros
				memset(tmp, 0, BYTES_PER_XDR_UNIT);
				for(i = last; i < len; i++)
					tmp[i-last] = (*ppsz)[i];

				if(!xdr_write(xdr, tmp, BYTES_PER_XDR_UNIT))
					return FALSE;
				}

			return TRUE;

		case XDR_DECODE:

			xdr_u_int(xdr, &len);	// read string length

			if(len > sizeMax)
				return FALSE;

			if(!*ppsz)	{			// allocate memory (if requested)
				if(len == RNDUP(len))
					*ppsz = (CHAR *)malloc(len+1);
				else
					*ppsz = (CHAR *)malloc(RNDUP(len));
				}

			if(!xdr_read(xdr, *ppsz, RNDUP(len)))
				return FALSE;
			(*ppsz)[len] = 0;

			return TRUE;

		case XDR_FREE:
			if(*ppsz)
				free(*ppsz);
			return TRUE;

		}
	return FALSE;
	}


/*************************************************************************
*
* Name	: xdr_opaque
*
* Descr.: handle an uninterpreted array of bytes
*
* Impl. : the byte-array is represented as <bytes[x]>, where x is a
*		  is a multiple of BYTES_PER_XDR_UNIT (4).
*
* Param.: CHAR *pb		- byte array
*		  UINT size		- size of byte array
*
* Return: BOOL fSuccess
*
*************************************************************************/
BOOL xdr_opaque(XDR *xdr, BYTE *pp, UINT size)	{
	UINT i, last;
	CHAR tmp[BYTES_PER_XDR_UNIT];

	switch(OP(xdr))	{
		case XDR_ENCODE:
			last = (size >> 2) << 2;
			if(!xdr_write(xdr, (CHAR *)pp, last))	// write full units
				return FALSE;

			if(last != size)	{	// fill last unit with zeros
				memset(tmp, 0, BYTES_PER_XDR_UNIT);
				for(i = last; i < size; i++)
					tmp[i-last] = pp[i];

				if(!xdr_write(xdr, tmp, BYTES_PER_XDR_UNIT))
					return FALSE;
				}

			return TRUE;

		case XDR_DECODE:
			if(!xdr_read(xdr, (CHAR *)pp, size))
				return FALSE;
			if(!xdr_read(xdr, tmp, RNDUP(size)-size))
				return FALSE;

			return TRUE;

		case XDR_FREE:
			return TRUE;

		}
	return FALSE;
	}


/*************************************************************************
*
* Name	: xdr_bytes
*
* Descr.: handle a vaiable length array of bytes
*
* Impl. : A byte array is represented as <UINT length><bytes[x]> where x
*		  is a multiple of BYTES_PER_XDR_UNIT (4).
*		  If '*ppb' is set to NULL at decoding, memory is allocated by
*		  this routine.
** Param.: CHAR **ppb	- pointer to string
*		  UINT *psize	- array size
*		  UINT sizeMax	- maximum string length
*
* Return: INT number of bytes read/written from/to the XDR stream
*
*************************************************************************/
BOOL xdr_bytes(XDR *xdr, BYTE **ppb, UINT *psize, UINT sizeMax)	{
	UINT len, i, last;
	CHAR tmp[BYTES_PER_XDR_UNIT];

	switch(OP(xdr))	{
		case XDR_ENCODE:
			if((!*ppb) || (*psize > sizeMax))
				return FALSE;

			xdr_u_int(xdr, psize);			// write length

			last = (*psize >> 2) << 2;
			if(!xdr_write(xdr, (CHAR *)*ppb, last))	// write full units
				return FALSE;

			if(last != *psize)	{	// fill last unit with zeros
				memset(tmp, 0, BYTES_PER_XDR_UNIT);
				for(i = last; i < *psize; i++)
					tmp[i-last] = (*ppb)[i];

				if(!xdr_write(xdr, tmp, BYTES_PER_XDR_UNIT))
					return FALSE;
				}

			return TRUE;

		case XDR_DECODE:

			xdr_u_int(xdr, psize);	// read length

			if(*psize > sizeMax)
				return FALSE;

			if(!*ppb)	{			// allocate memory (if requested)
				*ppb = (BYTE *)malloc(RNDUP(*psize));
				}

			return xdr_read(xdr, (CHAR *)*ppb, RNDUP(*psize));

		case XDR_FREE:
			if(*ppb)
				free(*ppb);
			return TRUE;

		}
	return FALSE;
	}


/*************************************************************************
*
* Name	: xdr_vector
*
* Descr.: handle a fixed-length array of objects
*
* Impl. : -
*
* Param.: XDR	*xdr	- XDR handle
*		  CHAR  *pa		- array pointer
*		  UINT	cObj	- number of objects in the array
*		  UINT	cbObj	- size of a single object
*		  xdrproc_t proc- procedure to handle an object
*
*************************************************************************/
BOOL xdr_vector(XDR *xdr, CHAR *pa, UINT cObj, UINT cbObj, xdrproc_t proc) {

	for(INT i  = 0; i < cObj; i++, pa += cbObj)	{
		if(!proc(xdr, (VOID *)pa))
			return FALSE;
		}

	return TRUE;
	}

/*************************************************************************
*
* Name	: xdr_array
*
* Descr.: handle a variable-length array of objects
*
* Impl. : If '*ppa' is set to NULL at decoding, memory is allocated by
*		  this routine.
*
* Param.: XDR	*xdr	- XDR handle
*		  CHAR  **ppa	- array pointer
*		  UINT	*pcObj	- number of objects in the array
*		  UINT  maxObj	- maximum number of objects
*		  UINT	cbObj	- size of a single object
*		  xdrproc_t proc- procedure to handle an object
*
*************************************************************************/
BOOL xdr_array(XDR *xdr, CHAR **ppa, UINT *pcObj, UINT maxObj,
											UINT cbObj, xdrproc_t proc) {
	INT i;

	switch(OP(xdr))	{
		case XDR_ENCODE:
			if(*pcObj > maxObj)
				return FALSE;

			if(!xdr_u_int(xdr, pcObj))	// write object counter
				return FALSE;
			break;

		case XDR_DECODE:
			if(!xdr_u_int(xdr, pcObj))	// read object counter
				return FALSE;

			if(*pcObj > maxObj)
				return FALSE;

			if(!*ppa)
				*ppa = (CHAR *)malloc(*pcObj * cbObj);
			break;

		case XDR_FREE:
			if(*ppa)
				free(*ppa);
			return TRUE;
		}
										// encode/decode each object
	for(i = 0; i < *pcObj; i++, (*ppa) += cbObj)
		if(!proc(xdr, (VOID *)*ppa))
			return FALSE;

	return TRUE;
	}


/*************************************************************************
*
* Name	: xdr_union
*
* Descr.: handle a discriminated union of objects
*
* Impl. : -
*
* Param.: XDR	*xdr	- XDR handle
*		  INT   *pDiscr - discriminator
*		  CHAR  *pUnion	- pointer to union
*		  xdr_discrim * - list of discriminators and their xdr-procs
*		  xdrproc_t def - procedure if no discriminator matches
*
*************************************************************************/
BOOL xdr_union(XDR *xdr, INT *pDiscr, CHAR *pUnion, xdr_discrim *choices,
														xdrproc_t defaultarm) {
	xdr_discrim *arm;

	xdr_int(xdr, pDiscr);			// write/read discriminator

	for(arm = choices; arm->discr && arm->discr != *pDiscr; arm++)
		;

	if(arm->discr)
		return arm->proc(xdr, pUnion);

	return (defaultarm)? defaultarm(xdr, pUnion) : FALSE;
	}


/*************************************************************************
*
* Name	: xdr_reference
*
* Descr.: handle a reference to an objects
*
* Impl. : -
*
* Param.: XDR	*xdr	- XDR handle
*		  CHAR  **pp	- object pointer
*		  UINT	cbObj	- size of a single object
*		  xdrproc_t proc- procedure to handle an object
*
*************************************************************************/
BOOL xdr_reference(XDR *xdr, CHAR **pp, UINT cbObj, xdrproc_t proc) {

	switch(OP(xdr))	{
		case XDR_ENCODE:
			if(!*pp)
				return FALSE;
			break;

		case XDR_DECODE:
			if(!*pp)
				*pp = (CHAR *)malloc(cbObj);
			break;

		case XDR_FREE:
			if(*pp)
				free(*pp);
			return TRUE;
		}

	return proc(xdr, (VOID *)*pp);
	}


/*************************************************************************
*
* Name	: xdr_pointer
*
* Descr.: handle linked lists of objects
*
* Impl. : -
*
* Param.: XDR	*xdr	- XDR handle
*		  CHAR  **pp	- pointer to next object
*		  UINT	cbObj	- size of a single object
*		  xdrproc_t proc- procedure to handle an object
*
*************************************************************************/
BOOL xdr_pointer(XDR *xdr, CHAR **pp, UINT cbObj, xdrproc_t proc) {
	INT fNext;

	fNext = (*pp)? TRUE : FALSE;	// needed only on XDR_ENCODE

	xdr_enum(xdr, &fNext);
	if(fNext)
		return xdr_reference(xdr, pp, cbObj, proc);

	return TRUE;
	}


/*************************************************************************
*
* Name	: xdr_free
*
* Descr.: free memory allocated by this library
*
* Impl. : -
*
* Param.: xdrproc_t 	- procedure that allocated the memory buffer
*		  CHAR  *p		- pointer to memory object
*
*************************************************************************/
VOID xdr_free(xdrproc_t proc, CHAR *p)	{

	if((proc == (xdrproc_t)xdr_array) ||
						(proc == (xdrproc_t)xdr_bytes)		||
						(proc == (xdrproc_t)xdr_pointer)	||
						(proc == (xdrproc_t)xdr_reference)	||
						(proc == (xdrproc_t)xdr_string)		||
						(proc == (xdrproc_t)xdr_vector))
		if(p)
			free(p);
	}

