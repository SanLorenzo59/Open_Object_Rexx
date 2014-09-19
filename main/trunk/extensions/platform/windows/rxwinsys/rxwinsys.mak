#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* http://www.oorexx.org/license.html                                         */
#/*                                                                            */
#/* Redistribution and use in source and binary forms, with or                 */
#/* without modification, are permitted provided that the following            */
#/* conditions are met:                                                        */
#/*                                                                            */
#/* Redistributions of source code must retain the above copyright             */
#/* notice, this list of conditions and the following disclaimer.              */
#/* Redistributions in binary form must reproduce the above copyright          */
#/* notice, this list of conditions and the following disclaimer in            */
#/* the documentation and/or other materials provided with the distribution.   */
#/*                                                                            */
#/* Neither the name of Rexx Language Association nor the names                */
#/* of its contributors may be used to endorse or promote products             */
#/* derived from this software without specific prior written permission.      */
#/*                                                                            */
#/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
#/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
#/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
#/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
#/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
#/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
#/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
#/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
#/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
#/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
#/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
#/*                                                                            */
#/*----------------------------------------------------------------------------*/

# NMAKE-compatible MAKE file for rxwinsys.dll

all:  $(OR_OUTDIR)\rxwinsys.dll $(OR_OUTDIR)\winsystm.cls

!include "$(OR_LIBSRC)\ORXWIN32.MAK"

SOURCE_DIR = $(OR_EXTENSIONS)\platform\windows\rxwinsys

C=cl
OPTIONS= $(cflags_common) $(OR_ORYXINCL)
OR_LIB=$(OR_OUTDIR)

SOURCEF=$(OR_OUTDIR)\rxwinsys.obj

# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(SOURCE_DIR)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

$(OR_OUTDIR)\rxwinsys.dll:     $(SOURCEF)
    $(OR_LINK) \
        $(SOURCEF)  \
    $(OR_OUTDIR)\verinfo.res \
    $(lflags_common) $(lflags_dll) \
    $(OR_LIB)\rexx.lib \
    $(OR_LIB)\rexxapi.lib shlwapi.lib \
    -def:$(SOURCE_DIR)\rxwinsys.def \
    -out:$(OR_OUTDIR)\$(@B).dll

#
# Copy winsystm.cls to the build directory so the test suite can be run directly
# from that location without doing an install.
#
$(OR_OUTDIR)\winsystm.cls : $(SOURCE_DIR)\winsystm.cls
    @ECHO .
    @ECHO Copying $(SOURCE_DIR)\winsystm.cls
    copy $(SOURCE_DIR)\winsystm.cls $(OR_OUTDIR)
