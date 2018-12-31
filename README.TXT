OS/2 XDR library
================

WHAT'S XDR
  The eXternal Data Representation standard is a standard for
  representating data in a machine-independent format. 
  It's very useful for transmitting data over heterogenous
  networks, where computers may have different byte-sex etc.


WHAT'S IN THIS LIBRARY
  This library contains an C++ implementation of the External Data
  Representation (XDR) standard for OS/2. (No OS/2 API calls have 
  been used, so it can be even used for other operating systems.)
  
  It is supposed that your computer is a little endian and
  your operating system uses ASCII-code for character strings.

  The programming interface was taken from SUN MICROSYSTEM'S
  XDR implementation. If you've access to SunOS or Solaris, you 
  can use 'rpcgen' to generate custom filters.
  If you're not programming C++ just rename XDR.CPP to XDR.C
  and it should work (I hope so). 


WHAT'S MISSING:
  Record streams and In-line makros are not implemented.


CONTENTS OF XDR.ZIP:
  read.me 	- this file
  xdr.h		- XDR header file (tab-size = 4)
  xdr.cpp	- XDR implementation (tab-size = 4)
  xdr.lib	- xdr.cpp compiled as a library
  xdrtest.cpp	- sample program


HOW TO USE THIS LIBRARY:
  Include XDR.H and link with XDR.LIB.
  or
  Include XDR.H and add XDR.CPP to your project.


WARNING:
  Using this library as part of DOS or WINDOS programs may destroy 
  your disk controller!


USAGE AND DISTRIBUTION:
  Anyone is free to use and change this library. If someone does
  major changes I would be glad to hear about it. Even if you've
  suggestions for improvements, just send me a mail:
	Compuserve: 100271,774
	E-Mail	  : caumanns@cs.tu-berlin.de
	
