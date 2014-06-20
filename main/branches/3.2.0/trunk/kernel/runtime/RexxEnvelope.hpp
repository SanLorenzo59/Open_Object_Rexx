/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                                  RexxEnvelope.hpp   */
/*                                                                            */
/* Primitive Envelope Class Definitions                                       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxEnvelope
#define Included_RexxEnvelope

/* The following bits define the type of envelope we are creating */
#define MOBILE_ENVELOPE 0x00000001     /* This envelope will be mobile  a mobi*/
#define METHOD_ENVELOPE 0x00000010     /* This envelope used to preserve      */
                                       /* a translated method in EA's         */

#define BEHAVIOUR_NON_PRIMITIVE 0x80000000
#define ObjectHasNonPrimitiveBehaviour(o) (((long)(((RexxObject *)o)->behaviour)) & BEHAVIOUR_NON_PRIMITIVE)

 class RexxEnvelope : public RexxObject {
  public:
   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   RexxEnvelope();
   inline RexxEnvelope(RESTORETYPE restoreType) { ; };
   RexxObject *execute();
   void live();
   void liveGeneral();
   void flatten(RexxEnvelope*);
   RexxObject *unflatten(RexxEnvelope *);
   void flattenReference(RexxObject **, LONG, RexxObject **);
   RexxEnvelope *pack(RexxString *, RexxObject *, RexxString *, RexxArray *);
   RexxObject *unpack();
   void        puff(RexxBuffer *, PCHAR);
   RexxObject *queryObj(RexxObject *);
   RexxObject *queryProxy(RexxObject *);
   RexxObject *copyBuffer(RexxObject *);
   void rehash();
   ULONG queryType();
   PCHAR  bufferStart();
   void    associateProxy(RexxObject *o ,RexxObject *p);
   void    addTable(RexxObject *obj);
   void    addProxy(RexxObject *o, RexxObject *p);

   inline RexxSmartBuffer *getBuffer() {return this->buffer;}
   inline RexxObject *getReceiver() {return this->receiver;}
   inline LONG        getCurrentOffset() { return this->currentOffset; }
   inline RexxObjectTable *getDuptable() {return this->duptable;}
   inline RexxObjectTable *getRehashtable() {return this->rehashtable;}

   RexxObject *home;
   RexxString *destination;            /* mailing destination               */
   RexxObject *receiver;               /* object to receive the message     */
   RexxString *message;                /* message to issue                  */
   RexxArray  *arguments;              /* array of arguments                */
   RexxObject *result;                 /* returned result                   */
   RexxObjectTable  *duptable;         /* table of duplicates               */
   RexxObjectTable  *savetable;        /* table of protected objects created during flattening */
   RexxSmartBuffer *buffer;            /* smart buffer wrapper              */
   RexxObjectTable  *rehashtable;      /* table to rehash                   */
   RexxStack  *flattenStack;           /* the flattening stack              */
   LONG        currentOffset;          /* current flattening offset         */
 };
#endif