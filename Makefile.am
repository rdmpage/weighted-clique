# $Id: Makefile.am,v 1.1.1.1 2007/10/28 08:36:53 rdmp1c Exp $

GRAPHDIR	= graph
LIBS		= -lGTL -L$(GRAPHDIR) -lgraph 
INCLUDES 	= -I$(GRAPHDIR) 

AM_CXXFLAGS=-Wno-deprecated
AM_CFLAGS=-Wno-deprecated

SUBDIRS = graph

# The actual programs
bin_PROGRAMS = equivalent
equivalent_SOURCES = equivalent.cpp


