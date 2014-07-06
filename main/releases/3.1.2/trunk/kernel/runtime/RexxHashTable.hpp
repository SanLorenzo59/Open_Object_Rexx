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
/* REXX Kernel                                                  RexxHashTable.hpp    */
/*                                                                            */
/* Primitive Hash Table Class Definitions                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxHash
#define Included_RexxHash

#define DEFAULT_HASH_SIZE  22L
#define STRING_TABLE    1
#define PRIMITIVE_TABLE 2
#define FULL_TABLE      3

/* The type for the reference links */
typedef unsigned long HashLink;

 typedef struct tabentry {
  RexxObject *value;                   /* item value object                 */
  RexxObject *index;                   /* item index object                 */
  HashLink next;                       /* next item in overflow bucket      */
 } TABENTRY;

 class RexxHashTable : public RexxInternalObject {
  public:

   inline void * operator new(size_t size, void *objectPtr) { return objectPtr; };
   inline RexxHashTable(RESTORETYPE restoreType) { ; };
   inline RexxHashTable() { ; }

   void         live();
   void         liveGeneral();
   void         flatten(RexxEnvelope *);
   RexxArray  * makeArray();

   HashLink       next(HashLink position);
   RexxObject    *value(HashLink position);
   RexxObject    *index(HashLink position);
   RexxObject    *merge(RexxHashTableCollection *target);
   RexxObject    *mergeItem(RexxObject *value, RexxObject *index);
   RexxHashTable *add(RexxObject *value, RexxObject *key);
   RexxObject    *remove(RexxObject *key);
   RexxArray     *getAll(RexxObject *key);
   RexxObject    *get(RexxObject *key);
   RexxHashTable *put(RexxObject *value, RexxObject *key);
   RexxHashTable *primitiveAdd(RexxObject *value, RexxObject *key);
   RexxObject    *primitiveRemove(RexxObject *key);
   RexxArray     *primitiveGetAll(RexxObject *key);
   RexxObject    *primitiveGet(RexxObject *key);
   RexxHashTable *primitivePut(RexxObject *value, RexxObject *key);
   RexxObject    *primitiveRemoveItem(RexxObject *value, RexxObject *key);
   RexxObject    *primitiveHasItem(RexxObject *, RexxObject *);
   size_t         totalEntries();
   HashLink       first();
   RexxObject    *replace(RexxObject *value, HashLink position);
   RexxArray     *allIndex(RexxObject *key);
   RexxObject    *getIndex(RexxObject *value);
   RexxHashTable *reHash();
   RexxHashTable *putNodupe(RexxObject *value, RexxObject *key);
   RexxArray     *values();
   RexxSupplier  *supplier();
   RexxObject    *removeItem(RexxObject *value, RexxObject *key);
   RexxObject    *stringGet(RexxString *key);
   RexxHashTable *stringPut(RexxObject *value, RexxString *key);
   RexxHashTable *stringAdd(RexxObject *value, RexxString *key);
   RexxArray     *stringGetAll(RexxString *key);
   RexxObject    *stringMerge(RexxHashTable *target);
   RexxObject    *hasItem(RexxObject * value, RexxObject *key);
   void           reMerge(RexxHashTable *target);
   void           primitiveMerge(RexxHashTable *target);
   RexxHashTable *insert(RexxObject *value, RexxObject *index, HashLink position, LONG type);
   RexxObject    *nextItem(RexxObject *, RexxObject *);
   inline size_t  mainSlotsSize()  { return this->u_size; };
   inline size_t  totalSlotsSize() { return this->u_size * 2; };
   inline BOOL    available(HashLink position) { return (size_t)position < this->totalSlotsSize(); };
   inline HashLink hashIndex(RexxObject *obj) { return (HashLink)(obj->hash() % this->mainSlotsSize()); }
   inline HashLink hashStringIndex(RexxObject *obj) { return (HashLink)(obj->hash() % this->mainSlotsSize()); }
//   inline HashLink hashStringIndex(RexxString *obj) { ULONG hash HASHVALUE((RexxObject *)obj);  size_t slotSize = mainSlotsSize(); return (hash > slotSize) ? hash % slotSize : ~hash % slotSize; }

   HashLink free;                      /* first free element                */
   TABENTRY entries[1];                /* hash table entries                */
 };
 #endif