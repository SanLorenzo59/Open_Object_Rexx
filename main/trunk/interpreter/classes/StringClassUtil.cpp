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
/* REXX Kernel                                                                */
/*                                                                            */
/* Argument-related utilities for some common string argument types.          */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ActivityManager.hpp"
#include "MethodArguments.hpp"


/**
 * Take in an agument passed to a method, convert it to a length
 * object, verifying that the number is a non-negative value.
 * If the argument is omitted, an error is raised.
 *
 * @param argument The argument reference to test.
 * @param position The position of the argument (used for error reporting.)
 *
 * @return The argument converted to a non-negative integer value.
 */
size_t lengthArgument(RexxInternalObject *argument, size_t position )
{
    if (argument == OREF_NULL)
    {
        missingArgument(position);
    }
    size_t    value;
    // converted using the ARGUMENT_DIGITS value
    if (!argument->unsignedNumberValue(value, Numerics::ARGUMENT_DIGITS))
    {
        reportException(Error_Incorrect_method_length, (RexxObject *)argument);
    }
    return value;
}


/**
 * Take in an agument passed to a method, convert it to a length
 * object, verifying that the number is a non-negative value.
 * If the argument is omitted, an error is raised.
 *
 * @param argument The argument reference to test.
 * @param position The position of the argument (used for error reporting.)
 *
 * @return The argument converted to a non-negative integer value.
 */
size_t nonNegativeArgument(RexxInternalObject *argument, size_t position )
{
    if (argument == OREF_NULL)
    {
        missingArgument(position);
    }

    return argument->requiredNonNegative(ARG_ONE, Numerics::ARGUMENT_DIGITS);
}


/**
 * Take in an agument passed to a method, convert it to a position
 * value, verifying that the number is a positive value.
 * If the argument is omitted, an error is raised.
 *
 * @param argument The argument to test.
 * @param position The argument list position of the argument.
 *
 * @return The converted numeric value.
 */
size_t positionArgument(RexxInternalObject *argument, size_t position )
{
    if (argument == OREF_NULL)
    {
        missingArgument(position);
    }
    size_t    value;

    if (!argument->unsignedNumberValue(value, Numerics::ARGUMENT_DIGITS) || value == 0)
    {
        reportException(Error_Incorrect_method_position, (RexxObject *)argument);
    }
    return value;
}


/**
 * Take in an argument passed to the BIF, convert it to a
 * character, if it exists otherwise return the default
 * character as defined (passed in) by the BIF.
 *
 * @param argument The argument to test.
 * @param position The argument position in the argument list.
 *
 * @return The first character of the option.
 */
char padArgument(RexxInternalObject *argument, size_t position)
{
    RexxString *parameter = (RexxString *)stringArgument(argument, position);
    // pad characters must be a single character long
    if (parameter->getLength() != 1)
    {
        reportException(Error_Incorrect_method_pad, (RexxObject *)argument);
    }
    return parameter->getChar(0);
}


/**
 * Take in an argument passed to the BIF, convert it to a
 * character, if it exists otherwise return the default
 * character as defined (passed in) by the BIF.
 *
 * @param argument The argument to test.
 * @param position The position of the argument
 *
 * @return The first character of the option string.
 */
char optionArgument(RexxInternalObject *argument, size_t position)
{
    // must be a string value
    RexxString *parameter = (RexxString *)stringArgument(argument, position);
    return toupper(parameter->getChar(0));
}


/**
 * Take in an argument passed to the BIF, convert it to a
 * character, if it exists otherwise return the default
 * character as defined (passed in) by the BIF.  Also validate
 * against the set of allowed characters.
 *
 * @param argument The argument to test.
 * @param position The position of the argument
 *
 * @return The first character of the option string.
 */
char optionArgument(RexxInternalObject *argument, const char *validOptions, size_t position)
{
    // must be a string value
    RexxString *parameter = (RexxString *)stringArgument(argument, position);

    // get the first character of the string
    char option = toupper(parameter->getChar(0));
    // if not one of the valid options (null string is not valid), raise the error
    if (parameter->isNullString() || strchr(validOptions, option) == NULL)
    {
        reportException(Error_Incorrect_method_option, validOptions, option);
    }
    return option;
}
