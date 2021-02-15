<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   <xsl:output method="text"/>

   <xsl:template match="Messages">
   <xsl:text>
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/*             -- Message resource file for ooRexx interpreter                */
/******************************************************************************/

STRINGTABLE
BEGIN

/* ------------------------------------------------------------------------
 * --    NOTE:   file is generated by build process
 * --            ==================================================
 * --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!
 * --            ==================================================
 * ------------------------------------------------------------------------
 */
</xsl:text>
   <xsl:for-each select="Message | Message/Subcodes/SubMessage">
       <xsl:sort select="MessageNumber" data-type="number"/>
<xsl:value-of select="Code"/> <xsl:value-of select="Subcode"/><xsl:text>    </xsl:text>&quot;<xsl:apply-templates select="Text"/>&quot;
   </xsl:for-each>
<xsl:text>
END

1 VERSIONINFO
 FILEVERSION ORX_VER,ORX_REL,ORX_MOD,ORX_BLD
 PRODUCTVERSION ORX_VER,ORX_REL,ORX_MOD,ORX_BLD
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x4L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "Rexx Language Association\0"
            VALUE "FileDescription", "Open Object Rexx Interpreter (main)\0"
            VALUE "FileVersion", ORX_VER_STR "\0"
            VALUE "InternalName", "REXX\0"
            VALUE "LegalCopyright", "Copyright © RexxLA " OOREXX_COPY_YEAR ".\0"
            VALUE "OriginalFilename", "REXX.DLL\0"
            VALUE "ProductName", "Open Object Rexx for Windows\0"
            VALUE "ProductVersion",  ORX_VER_STR "\0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END

/* -------------------------------------------------------------------------- */
/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */
   </xsl:text>
   </xsl:template>

   <xsl:template match="Text">
       <xsl:apply-templates/>
   </xsl:template>

   <xsl:template match="q">
       <xsl:text>&quot;&quot;</xsl:text><xsl:apply-templates/><xsl:text>&quot;&quot;</xsl:text>
   </xsl:template>

   <xsl:template match="sq">
       <xsl:text>&apos;</xsl:text>
   </xsl:template>

   <xsl:template match="dq">
       <xsl:text>&quot;&quot;</xsl:text>
   </xsl:template>

   <xsl:template match="Sub">
       <xsl:text>&amp;</xsl:text><xsl:value-of select="@position"/>
   </xsl:template>
</xsl:stylesheet>
