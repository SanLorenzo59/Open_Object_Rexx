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
# NMAKE-compatible MAKE file to build REXX sample program callrxwn.exe.
# This example is a Windows mode (GUI) application.
#
# Notes:
#  - Make sure the path to rexx.lib and rexxapi.lib is in your LIB environment
#    variable
#  - Make sure the path to rexx.h is in your INCLUDE environment variable
#  - Uses /D_CRT_SECURE_NO_DEPRECATE to eliminate deprecation warnings under
#    MS Visual Studio 2005

TARGET=callrxwn

all: $(TARGET).exe


callrxwn.exe:     callrxwn.obj callrxwn.res
    link     callrxwn.obj callrxwn.res \
             /NOLOGO /SUBSYSTEM:Windows user32.lib gdi32.lib winspool.lib \
             comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib    \
             shell32.lib kernel32.lib \
             rexx.lib rexxapi.lib


callrxwn.obj:     callrxwn.c
    @ECHO Compiling $(@B).c
     cl callrxwn.c \
     /nologo /D:_X86_ /DWIN32 /D_CRT_SECURE_NO_DEPRECATE /W3 -c


callrxwn.res: callrxwn.rc callrxwn.h
    @ECHO ResourceCompiling $(@B).res
        rc /DWIN32 -r -fo callrxwn.res callrxwn.rc