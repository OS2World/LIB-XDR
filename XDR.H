/*************************************************************************
*
* File Name	 : XDR.H
*
* Description: OS/2 XDR library header file
*			   For further information about XDR see:
*				- RFC 1014
*			    - John R. Corbin: The Art of Distributed Applications,
*				  Springer Verlag, 1991
*
* Data Types : XDR			- XDR stream handle
*			   XDR_OP		- XDR options (encode, decode, free)
*			   xdrproc_t    - XDR decoding/encoding procedure
*			   xdr_discrim	- XDR discriminated union structure
*
* Procedures : xdrstdio_create()   	xdrmem_create()		xdr_destroy()
*			   xdr_free()			xdrmem_base()		xdr_getpos()
*			   xdr_setpos()			xdr_char()			xdr_short()
*			   xdr_u_short()		xdr_int()			xdr_u_int()
*			   xdr_long()			xdr_u_long()		xdr_void()
*			   xdr_enum()			xdr_string()		xdr_opaque()
*			   xdr_bytes()			xdr_vector()		xdr_array()
*			   xdr_union()
*
* copyright (c) J”rg Caumanns, 1993 [caumanns@cs.tu-berlin.de]
*
*      DISCLAIMER OF WARRANTIES.
*      The code is provided "AS IS", without warranty of any kind.
*
*************************************************************************/
#ifndef _XDR_INCLUDED
#define _XDR_INCLUDED

#include <os2def.h>
#include <stdio.h>


/*
* XDR: dummy for the 'real' XDR-structure (defined in XDR.cpp)
*/
typedef struct XDR	{
	CHAR	dummy[32];
	};

/*
* XDR_OP: XDR options
*/
typedef enum XDR_OP {
			XDR_ENCODE,				// encode XDR stream
			XDR_DECODE,				// decode XDR stream
			XDR_FREE	};			// free memory allocated at decoding

/*
* xdrproc_t
*/
typedef BOOL(*xdrproc_t)(XDR *, VOID *);

/*
* xdr_discrim
*/
typedef struct xdr_discrim	{
	SHORT		discr;				// discriminator
	xdrproc_t   proc;				// XDR procedure for this arm of the union
	};


/*
* function prototypes
*/
VOID xdrstdio_create(XDR *, FILE *, XDR_OP);
VOID xdrmem_create(XDR *, CHAR *, ULONG, XDR_OP);
VOID xdr_destroy(XDR *);
VOID xdr_free(xdrproc_t proc, CHAR *);
CHAR *xdrmem_base(XDR *);
UINT xdr_getpos(XDR *);
BOOL xdr_setpos(XDR *, UINT);

BOOL xdr_char	(XDR *, CHAR *	);
BOOL xdr_short	(XDR *, SHORT *	);
BOOL xdr_u_short(XDR *, USHORT *);
BOOL xdr_int	(XDR *, INT *	);
BOOL xdr_u_int	(XDR *, UINT *	);
BOOL xdr_long	(XDR *, LONG *	);
BOOL xdr_u_long	(XDR *, ULONG *	);
BOOL xdr_void	(XDR *, VOID *	);
BOOL xdr_enum	(XDR *, INT *	);

BOOL xdr_string(XDR *, CHAR **, UINT);
BOOL xdr_opaque(XDR *, CHAR * , UINT);
BOOL xdr_bytes (XDR *, BYTE **, UINT *, UINT);
BOOL xdr_vector(XDR *, CHAR *,  UINT, 	UINT, xdrproc_t);
BOOL xdr_array (XDR *, CHAR **, UINT *, UINT, UINT,    xdrproc_t);
BOOL xdr_union (XDR *, INT  *,  CHAR *, xdr_discrim *, xdrproc_t);


#endif /* _XDR_INCLUDED */