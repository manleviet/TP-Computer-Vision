#
#  Exemple de Makefile
#
#  Alain Boucher
#
#  2/04/2003
#



SHELL	= 	/bin/sh
RM	=	/bin/rm -f
CP	=	/bin/cp

#---------------------------------------------------------
#
#  compilation variables
#

# repertoires des *.h supplementaires.  Ajouter -I
# avant chaque repertoire : exemple -I/usr/include/local
INCLUDEDIR	=  -I/usr/include/opencv
#-Iutil -Idistribution -Itraitements \
#				-Isegmentation -Ireconnaissance -Icovariance \
#				-Ipollen -Inumrec

# repertoire des librairies supplementaires.  Ajouter -L
# avant chaque repertoire : exemple -L/usr/local/lib
LIBDIR		=	

# liste des librairies necessaires pour l'edition des liens.
LIBS		=       -lcv -lhighgui

# compilateurs utilises
CC		=	gcc
CCC		=	g++

# options de compilation
CFLAGS		=	$(INCLUDEDIR) -Wall 

# options pour l'edition des liens
LFLAGS		=	$(LIBDIR)

# repertoire pour les executables
XDIR		= .


#---------------------------------------------------------
#
# Repertoires des differents modules
#

# Declarez ici une variable contenant les fichiers
# objets de votre application (tous les *.cpp mais
# avec l'extension *.o)
# attention :  mettre un <TAB> au debut de chaque ligne

OBJ-gen			=	basetextures.o \
	classtexture.o \
	glcm.o \
	knearest.o \
	texture.o\
	main.o


OBJ-exemple = $(OBJ-gen)



#---------------------------------------------------------

# regles de compilation automatique des fichiers

.SUFFIXES:	.o .cpp .c .cc

.cpp.o		:
		$(CCC) $(CFLAGS) -o $*.o -c $*.cpp

.cc.o		:
		$(CCC) $(CFLAGS) -o $*.o -c $*.cc

.c.o		:
		$(CC) $(CFLAGS) -o $*.o -c $*.c 

#---------------------------------------------------------

# regle d'edition des liens de votre application
# changez ici le mot prog pour mettre le nom de
# votre application
# Les fichier *.o sont declares dans la variable
# OBJ-prog

all : segtex

segtex: $(OBJ-exemple)
	$(CCC) $(LFLAGS) -o $(XDIR)/$(@F) $(OBJ-exemple) $(LIBS)



# -----------------------------------------------------------------------
depend		:
		makedepend $(INCLUDEDIR) *.cc */*.cc *.cpp */*.cpp *.c */*.c
      
clean		: 
		$(RM) -r *.o */*.o
