

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX  Support                                                              */
/*                                                                            */
/* Rexx primitive class id codes                                              */
/*                                                                            */
/*        -- DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST! --            */
/*                  -- FILE WAS GENERATED BY ClassTypeIds.xsl --              */
/******************************************************************************/

#ifndef ClassTypeCodes_Included
#define ClassTypeCodes_Included

typedef enum
{
    T_First_Primitive_Class = 0,
    T_First_Exported_Class = 0,

    T_Object = 0,
    T_ObjectClass = 1,
    T_Class = 2,
    T_ClassClass = 3,
    T_Array = 4,
    T_ArrayClass = 5,
    T_Directory = 6,
    T_DirectoryClass = 7,
    T_Integer = 8,
    T_IntegerClass = 9,
    T_List = 10,
    T_ListClass = 11,
    T_Message = 12,
    T_MessageClass = 13,
    T_Method = 14,
    T_MethodClass = 15,
    T_NumberString = 16,
    T_NumberStringClass = 17,
    T_Queue = 18,
    T_QueueClass = 19,
    T_Stem = 20,
    T_StemClass = 21,
    T_String = 22,
    T_StringClass = 23,
    T_Supplier = 24,
    T_SupplierClass = 25,
    T_Table = 26,
    T_TableClass = 27,
    T_StringTable = 28,
    T_StringTableClass = 29,
    T_Relation = 30,
    T_RelationClass = 31,
    T_MutableBuffer = 32,
    T_MutableBufferClass = 33,
    T_Pointer = 34,
    T_PointerClass = 35,
    T_Buffer = 36,
    T_BufferClass = 37,
    T_WeakReference = 38,
    T_WeakReferenceClass = 39,
    T_Routine = 40,
    T_RoutineClass = 41,
    T_Package = 42,
    T_PackageClass = 43,
    T_RexxContext = 44,
    T_RexxContextClass = 45,
    T_IdentityTable = 46,
    T_IdentityTableClass = 47,
    T_StackFrame = 48,
    T_StackFrameClass = 49,
    T_Set = 50,
    T_SetClass = 51,
    T_Bag = 52,
    T_BagClass = 53,
    T_RexxInfo = 54,
    T_RexxInfoClass = 55,

    T_Last_Exported_Class = 55,
    
    T_First_Internal_Class = 56,

    T_NilObject = 56,
    T_Behaviour = 57,
    T_MethodDictionary = 58,
    T_LibraryPackage = 59,
    T_RexxCode = 60,
    T_NativeMethod = 61,
    T_NativeRoutine = 62,
    T_RegisteredRoutine = 63,
    T_CPPCode = 64,
    T_AttributeGetterCode = 65,
    T_AttributeSetterCode = 66,
    T_ConstantGetterCode = 67,
    T_AbstractCode = 68,
    T_SmartBuffer = 69,
    T_IdentityHashContents = 70,
    T_EqualityHashContents = 71,
    T_MultiValueContents = 72,
    T_StringHashContents = 73,
    T_ListContents = 74,
    T_Variable = 75,
    T_VariableDictionary = 76,
    T_VariableTerm = 77,
    T_CompoundVariableTerm = 78,
    T_StemVariableTerm = 79,
    T_DotVariableTerm = 80,
    T_IndirectVariableTerm = 81,
    T_FunctionCallTerm = 82,
    T_MessageSendTerm = 83,
    T_UnaryOperatorTerm = 84,
    T_BinaryOperatorTerm = 85,
    T_LogicalTerm = 86,
    T_ListTerm = 87,
    T_Instruction = 88,
    T_AddressInstruction = 89,
    T_AssignmentInstruction = 90,
    T_CallInstruction = 91,
    T_DynamicCallInstruction = 92,
    T_CallOnInstruction = 93,
    T_CommandInstruction = 94,
    T_SimpleDoInstruction = 95,
    T_DoForeverInstruction = 96,
    T_DoOverInstruction = 97,
    T_DoOverUntilInstruction = 98,
    T_DoOverWhileInstruction = 99,
    T_DoOverForInstruction = 100,
    T_DoOverForUntilInstruction = 101,
    T_DoOverForWhileInstruction = 102,
    T_ControlledDoInstruction = 103,
    T_ControlledDoUntilInstruction = 104,
    T_ControlledDoWhileInstruction = 105,
    T_DoWhileInstruction = 106,
    T_DoUntilInstruction = 107,
    T_DoCountInstruction = 108,
    T_DoCountUntilInstruction = 109,
    T_DoCountWhileInstruction = 110,
    T_DropInstruction = 111,
    T_ElseInstruction = 112,
    T_EndInstruction = 113,
    T_EndIfInstruction = 114,
    T_ExitInstruction = 115,
    T_ExposeInstruction = 116,
    T_ForwardInstruction = 117,
    T_GuardInstruction = 118,
    T_IfInstruction = 119,
    T_CaseWhenInstruction = 120,
    T_InterpretInstruction = 121,
    T_LabelInstruction = 122,
    T_LeaveInstruction = 123,
    T_MessageInstruction = 124,
    T_NopInstruction = 125,
    T_NumericInstruction = 126,
    T_OptionsInstruction = 127,
    T_OtherwiseInstruction = 128,
    T_ParseInstruction = 129,
    T_ProcedureInstruction = 130,
    T_QueueInstruction = 131,
    T_RaiseInstruction = 132,
    T_ReplyInstruction = 133,
    T_ReturnInstruction = 134,
    T_SayInstruction = 135,
    T_SelectInstruction = 136,
    T_SelectCaseInstruction = 137,
    T_SignalInstruction = 138,
    T_DynamicSignalInstruction = 139,
    T_SignalOnInstruction = 140,
    T_ThenInstruction = 141,
    T_TraceInstruction = 142,
    T_UseInstruction = 143,
    T_ClassDirective = 144,
    T_LibraryDirective = 145,
    T_RequiresDirective = 146,
    T_CompoundElement = 147,
    T_ParseTrigger = 148,
    T_ProgramSource = 149,
    T_ArrayProgramSource = 150,
    T_BufferProgramSource = 151,
    T_FileProgramSource = 152,
    T_NumberArray = 153,
    T_ClassResolver = 154,

    T_Last_Internal_Class = 154,
    
    T_First_Transient_Class = 155,

    T_Memory = 155,
    T_InternalStack = 156,
    T_LiveStack = 157,
    T_PushThroughStack = 158,
    T_Activity = 159,
    T_Activation = 160,
    T_NativeActivation = 161,
    T_ActivationFrameBuffer = 162,
    T_Envelope = 163,
    T_LanguageParser = 164,
    T_Clause = 165,
    T_Token = 166,
    T_DoBlock = 167,
    T_InterpreterInstance = 168,
    T_SecurityManager = 169,
    T_CommandHandler = 170,
    T_MapBucket = 171,
    T_MapTable = 172,
    T_TrapHandler = 173,

    T_Last_Transient_Class = 173,
    T_Last_Primitive_Class = 173,
    T_Last_Class_Type = 173,
    
} ClassTypeCode;

/* -------------------------------------------------------------------------- */
/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */
#endif

