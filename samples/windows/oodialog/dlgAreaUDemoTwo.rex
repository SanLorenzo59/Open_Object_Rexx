/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2011 Rexx Language Association. All rights reserved.    */
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

/**
 *  DlgAreaDemoTwo.Rex
 *
 *  Demonstrates a second approach to resizable dialogs.  Essentially what this
 *  approach does is to defer the redrawing of the dialog controls until the
 *  user has finished resizing the dialog.
 *
 *  By default, the DialogAreaU class calls a dialog method (the update method)
 *  that forces all the dialog controls to redraw themselves every time a resize
 *  event ocurrs.  This causes the constant flicker seen in DlgAreaDemo.rex.
 *
 *  The approach taken here is to tell the DialogAreaU object to *not* invoke
 *  the update method during the resize event.  This is done by setting the
 *  updateOnResize attribute to false in the DialogAreaU object.
 *
 *  Then we connect the size / move ended event to a method in our dialog.  This
 *  event is called exactly once when the user has stopped resizing or moving
 *  the dialog.
 *
 *  We keep track of whether the user is resizing, or not.  When we get the size
 *  move ended event, if the user was resizing, we invoked the update method of
 *  the dialog, forcing all the dialog controls to redraw themselves in their
 *  new, final, position.
 *
 *  This eliminates the flicker, but also makes it appear as though the dialog
 *  controls are not changing while the user is actively resizing.  When the
 *  user stops resizing, the dialog controls "magically" appear in their new
 *  size and position.
 *
 *  Which approach is better is probably a matter of personal preference.
 */

  myDlg = .MyDialog~new
  myDlg~execute('ShowTop')

  return 0

::requires "ooDialog.cls"

::class 'MyDialog' subclass UserDialog

::method init

  self~init:super
  self~createCenter(250, 250, 'MyDialog',                             -
                              'ThickFrame MinimizeBox MaximizeBox', , -
                              'MS Sans Serif', 8)

  self~connectResize('onResize')
  self~connectSizeMoveEnded('onSizeMoveEnded')


::method defineDialog
  expose u sizing

  u = .dlgAreaU~new(self)                                            /* whole dlg   */
  if u~lastError \= .nil then call errorDialog u~lastError

  -- Tell the DialogAreaU object to not invoke the update method.  We are not
  -- resizing now.
  u~updateOnResize = .false
  sizing = .false

  u~noResize~put(13)
  e = .dlgArea~new(u~x       , u~y       , u~w('70%'), u~h('90%'))   /* edit   area */
  s = .dlgArea~new(u~x       , u~y('90%'), u~w('70%'), u~hr      )   /* status area */
  b = .dlgArea~new(u~x('70%'), u~y       , u~wr      , u~hr      )   /* button area */

  self~addEntryLine(12, 'text', e~x, e~y, e~w, e~h, 'multiline')
  self~addText(s~x, s~y, s~w, s~h, 'Status info appears here', , 11)

  self~addButton(13, b~x, b~y('00%'), b~w, b~h('9%'), 'Button' 0 , 'Button'||0)
  self~addButton(14, b~x, b~y('10%'), b~w, b~h('9%'), 'Button' 1 , 'Button'||1)
  self~addButton(15, b~x, b~y('20%'), b~w, b~h('9%'), 'Button' 2 , 'Button'||2)
  self~addButton(16, b~x, b~y('30%'), b~w, b~h('9%'), 'Button' 3 , 'Button'||3)
  self~addButton(17, b~x, b~y('40%'), b~w, b~h('9%'), 'Button' 4 , 'Button'||4)
  self~addButton(18, b~x, b~y('50%'), b~w, b~h('9%'), 'Button' 5 , 'Button'||5)
  self~addButton(19, b~x, b~y('60%'), b~w, b~h('9%'), 'Button' 6 , 'Button'||6)
  self~addButton( 1, b~x, b~y('90%'), b~w, b~h('9%'), 'Ok',        'Ok', 'DEFAULT')


::method initDialog

  -- The underlying edit controls internally resize themselves as the dialog
  -- they are contained in is resized.  We don't want that, so we disable that
  -- behavior in the underlying edit control.
  self~newEdit(12)~disableInternalResize


::method onResize unguarded
  expose u sizing
  use arg dummy, sizeinfo

  -- We are resizing now.
  sizing = .true

  u~resize(self, sizeinfo)
  return 0


::method onSizeMoveEnded unguarded
  expose sizing

  -- If we were resizing, force the dialog controls to redraw themselves.
  if sizing then self~update

  -- We are not resizing.
  sizing = .false
  return 0


::method unknown
  use arg msgname, args
  self~newStatic(11)~setText('You Pressed' msgname)
