/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
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
/* ooDialog User Guide
   Exercise 04b: ProductView.rex - The ProductView component      v00-04 17Aug11

   Contains: 	   classes "ProductView" and "AboutDialog".
   Pre-requisites: ProductView.dll, ProductView.h.
   		   Support\NumberOnlyEditEx.cls (copied from ooDialog Samples)

   Description: A sample Product View component - part of the sample
        	Order Management application.

   Outstanding Problems: None reported.

   Changes:
   v00-01 21Jly11
   v00-02 28Jly11: Added a constants class for user-visible messages.
   v00-03 06Aug11: corrected typos in some comments.
   		   deleted a commented-out statement that shouldn't have been
                   left in the file, plus another typo in a comment corrected.
                   use of a ProductDT rather than directory for data.
                   Changed class name of strings to HRS.
   v00-04 17Aug11: Changed to .Application~setDefaults in newInstance method,
                   and deleted autoDetection methods - not now needed as turn
                   off autoDetection in .Application~setDefaults().
------------------------------------------------------------------------------*/

::requires "ooDialog.cls"
::requires "..\Support\NumberOnlyEditEx.cls"
::requires "ProductModelData.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductView							  v01-00 12Jly11
  -----------
  The "view" part of the Product component.
  [interface (idl format)]
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductView SUBCLASS ResDialog PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC UNGUARDED
    say ".ProductView-newInstance-01: Start."
    -- Enable use of symbolic IDs in menu creation, and turn off AutoDetection
    -- (the third parameter:
    .Application~setDefaults("O", "ProductView.h", .false)
    -- Create an instance of ProductView and show it:
    dlg = .ProductView~new("res\ProductView.dll", IDD_PRODUCT_VIEW)
    say ".ProductView-newInstance-02: dlg~Activate."
    dlg~activate


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    say "ProductView-init-01."
    -- called first (result of .ProductView~new)
    forward class (super) continue


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    say "ProductView-activate-01."
    self~execute("SHOWTOP","IDB_PROD_ICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar prodControls prodData
    say "ProductView-initDialog-01"

    menuBar = .BinaryMenuBar~new(self, IDR_PRODUCT_VIEW_MENU, , self, .true)

    prodControls = .Directory~new
    prodControls[ecProdNo]         = self~newEdit("IDC_PROD_NO")
    prodControls[ecProdName]       = self~newEdit("IDC_PROD_NAME")
    prodControls[ecProdPrice]      = self~newEdit("IDC_PROD_LIST_PRICE")
    prodControls[ecUOM]            = self~newEdit("IDC_PROD_UOM")
    prodControls[ecProdDescr]      = self~newEdit("IDC_PROD_DESCRIPTION")
    prodControls[gbSizes]          = self~newEdit("IDC_PROD_SIZE_GROUP")		-- Do we ever need this?
    prodControls[rbSmall]          = self~newRadioButton("IDC_PROD_RADIO_SMALL")
    prodControls[rbMedium]         = self~newRadioButton("IDC_PROD_RADIO_MEDIUM")
    prodControls[rbLarge]          = self~newRadioButton("IDC_PROD_RADIO_LARGE")
    prodControls[pbSaveChanges]    = self~newPushButton("IDC_PROD_SAVE_CHANGES")
    self~connectButtonEvent("IDC_PROD_SAVE_CHANGES","CLICKED",saveChanges)

    -- Use NumberOnlyEditEx.cls to enforce numeric only entry for Price and UOM:
    prodControls[ecProdPrice]~initDecimalOnly(2,.false)		-- 2 decimal places, no sign.
    prodControls[ecUOM]~initDecimalOnly(0,.false)		-- 0 decimal places, no sign.
    prodControls[ecProdPrice]~connectCharEvent(onChar)
    prodControls[ecUOM]~connectCharEvent(onChar)

    prodData = self~getData	-- Gets data from ProductModel into prodData
    self~showData	-- Show the data
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Event Handler Methods - MenuBar Events:
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD updateProduct UNGUARDED
    expose prodControls

    -- Enable the controls to allow changes to the data:
    prodControls[ecProdName]~setReadOnly(.false)
    prodControls[ecProdPrice]~setReadOnly(.false)
    prodControls[ecUOM]~setReadOnly(.false)
    prodControls[ecProdDescr]~setReadOnly(.false)
    prodControls[rbSmall]~enable
    prodControls[rbMedium]~enable
    prodControls[rbLarge]~enable
    self~enableControl("IDC_PROD_SAVE_CHANGES")
    prodControls[pbSaveChanges]~state = "FOCUS"  -- Put input focus on the button
    self~tabToNext()				 -- put text cursor on Product Description
    						 --   (as if the user had pressed tab)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD refreshData UNGUARDED
    self~disableControl("IDC_PROD_SAVE_CHANGES")
    self~showData

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD print UNGUARDED
    say "ProductView-print-01"

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD close UNGUARDED
    say "ProductView-close-01"
    return self~cancel:super

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    say "ProductView-about-01"
    dlg = .AboutDialog~new("ProductView.dll", IDD_PRODUCT_VIEW_ABOUT)
    dlg~execute("SHOWTOP")


  /*----------------------------------------------------------------------------
    Event Handler Methods - PushButton Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    "Save Changes" - Collects new data, checks if there has indeed been a
                     change, and if not, issues a warning msg. Disables the
                     button when valid changes made.                          */
  ::METHOD saveChanges UNGUARDED
    expose prodControls prodData

    -- Transform data from view format (as in controls) to app format (a directory):
    newProdData = self~xformView2App(prodControls)

    -- Check if anything's changed; if not, show msgbox and exit with controls enabled.
    -- If changed, go on to validate data.
    result = self~checkForChanges(newProdData)
    if result = .false then do
      msg = .HRS~nilSaved
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,.HRS~updateProd,"OK","WARNING","DEFBUTTON2 APPLMODAL")
      return
    end

    -- Now validate data:
    result = self~validate(newProdData)		-- returns a null string or error messages.
         					-- Better would be a set of error numbers.
    -- If no problems, then show msgbox and go on to disable controls.
    if result = "" then do
      msg = .HRS~saved
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,.HRS~updateProd,"OK","INFORMATION","DEFBUTTON1 APPLMODAL")
    end
    -- If problems, then show msgbox and leave user to try again or refresh or exit.
    else do
      msg = result||.EndOfLine||.HRS~notSaved
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,.HRS~updateProd,"OK","ERROR","DEFBUTTON1 APPLMODAL")
      return
    end

    -- Send new data to be checked by CustomerModel object (not implemented).

    -- Disable controls that were enabled by menu "ActionsFile-->Update" selection:
    prodControls[ecProdName]~setReadOnly(.true)
    prodControls[ecProdDescr]~setReadOnly(.true)
    prodControls[ecProdPrice]~setReadOnly(.true)
    prodControls[ecUom]~setReadOnly(.true)
    if newProdData~size \= "S" then prodControls[rbSmall]~disable
    if newProdData~size \= "M" then prodControls[rbMedium]~disable
    if newProdData~size \= "L" then prodControls[rbLarge]~disable
    self~disableControl("IDC_PROD_SAVE_CHANGES")

    prodData = newProdData
    prodData~list
    return

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Event Handler Methods - Keyboard Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD onChar UNGUARDED
    -- called for each character entered in the price or UOM fields.
    forward to (arg(6))

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    "OK" - This is a no-op method that over-rides the default Windows action
           of 'close window' for an Enter key 				    --*/
  ::METHOD ok unguarded
    return

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    "Cancel" - This is a no-op method that over-rides the default Windows action
           of 'cancel window' for an Escape key.			    --*/
  ::METHOD cancel
    return


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getData
    -- Get data from the ProductModel:
    --expose prodData
    say "ProductView-getData-01."
    idProductModel = .local~my.idProductModel
    prodData = idProductModel~query		-- prodData is of type ProductDT
    return prodData


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showData
    -- Transfrom data (where necessary) to display format, and then disable controls.
    expose prodControls prodData
    say "ProductView-showData-01."
    -- Set data in controls:
    prodControls[ecProdNo]~setText(   prodData~number       )
    prodControls[ecProdName]~setText( prodData~name         )
    -- Price in prodData has no decimal point - 2 decimal places are implied - hence /100 for display.
    prodControls[ecProdPrice]~setText(prodData~price/100    )
    prodControls[ecUOM]~settext(      proddata~uom          )
    prodControls[ecProdDescr]~setText(prodData~description  )
    size = prodData~size
    -- Disable controls
    prodControls[ecProdName]~setReadOnly(.true)
    prodControls[ecProdPrice]~setReadOnly(.true)
    prodControls[ecUOM]~setReadOnly(.true)
    prodControls[ecProdDescr]~setReadOnly(.true)
    prodControls[rbSmall]~disable
    prodControls[rbMedium]~disable
    prodControls[rbLarge]~disable
    -- But check correct button and enable it to highlight it to the user:
    select
      when size = "S" then do
        .RadioButton~checkInGroup(self,"IDC_PROD_RADIO_SMALL","IDC_PROD_RADIO_LARGE","IDC_PROD_RADIO_SMALL")
        prodcontrols[rbSmall]~enable
      end
      when size = "M" then do
        .RadioButton~checkInGroup(self,"IDC_PROD_RADIO_SMALL","IDC_PROD_RADIO_LARGE","IDC_PROD_RADIO_MEDIUM")
        prodcontrols[rbMedium]~enable
      end
      otherwise do
        .RadioButton~checkInGroup(self,"IDC_PROD_RADIO_SMALL","IDC_PROD_RADIO_LARGE","IDC_PROD_RADIO_LARGE")
        prodcontrols[rbLarge]~enable
      end
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD checkForChanges
    -- Check whether any data has actually changed when "Save Changes" button
    -- has been pressed. Return .true if data changed, else returns .false.
    expose prodData
    use arg newProdData

    changed = .false
    select
      when newProdData~name        \= prodData~name        then changed = .true
      when newProdData~price       \= prodData~price       then changed = .true
      when newProdData~uom         \= prodData~uom         then changed = .true
      when newProdData~description \= prodData~description then changed = .true
      when newProdData~size        \= ProdData~size        then changed = .true
      otherwise nop
    end
    return changed


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD validate
    -- Validation: 1. Check price/UOM not changed > 50% up or down.
    --             2. Cannot change from Large to Small without UOM increasing
    --                 by at least 100%; nor from Small to Large without
    --                 decreasing by more than 50%.
    --             3. Product Description <= 100 characters.
    --             4. Product Name <= 30 characters.
    --  Returns string of messages - the null string if all OK.
    expose prodData
    use arg newProdData
    msg = ""

    -- Check Price (catches decimal point errors also):
    price = prodData~price; newPrice = newProdData~price
    oldUom   = prodData~uom;       newUom   = newProdData~uom 	-- 'oldUom - avoids name conflict with 'uom' in newProddata~uom.
    if ((price/oldUom)*1.5 < newPrice/newUom) | (newPrice/newUom < (price/oldUom)/2) then do
      msg = msg||.HRS~badRatio||" "
    end

    -- Check Size vs UOM:
    if prodData~size = "L" & newProdData~size = "S" -    -- Large to Small
        & prodData~uom/2 < newProdData~uom then do
      msg = msg||.HRS~uomTooBig||" "
    end
      if prodData~size = "S" & newProdData~size = "L" -    -- Small to Large
        & prodData~uom*2 > newProdData~uom then do
      msg = msg||.HRS~uomTooSmall||" "
    end

    -- Check Product Description length:
    if newProdData.description~length > 80 then do
      msg = msg||.HRS~descrTooBig||" "
    end

    -- Check Product Name length:
    if newProdData~name~length > 30 then do
      msg = msg||.HRS~prodNameTooBig
    end

    return msg


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD xformView2App
    -- Transforms Product Data from View form (in the GUI controls) to
    --   App form (a directory with address as an array).
    expose prodControls
    prodData = .ProductDT~new
    prodData~number = prodControls[ecProdNo]~gettext()
    prodData~name = prodControls[ecProdName]~getText()
    price = prodControls[ecProdPrice]~getText()
    -- Data entered has or assumes a decimal point; but data in "application"
    --   is a whole number (e.g. $42.42 is recorded in the database as "4242").
    --   So re-format data from decimal to whole number:
    priceTwoDecs = price~format(,2)		-- force 2 dec positions
    -- Re-display price properly formatted (in case the user did not format correctly - e.g. entered "42" or "38.4"):
    prodControls[ecProdPrice]~setText(priceTwoDecs)
    -- Now format price to "application" format:
    price = (priceTwoDecs*100)~format(,0)	-- multiply by 100 and then force whole number.
    prodData~price = price
    prodData~uom  = prodControls[ecUOM]~getText()
    prodData~description = prodControls[ecProdDescr]~getText()
    select
      when prodControls[rbSmall]~checked then prodData~size = "S"
      when prodControls[rbMedium]~checked then prodData~size = "M"
      otherwise prodData~size = "L"
    end

    return prodData

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  AboutDialog							v01-00 17May11
  -------------
  The "About" class - shows a dialog box that includes a bitmap - part of the
  ProductView component.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS AboutDialog SUBCLASS ResDialog

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method initDialog
    expose font image
    resImage = .ResourceImage~new( "", self)			  -- Create an instance of a resource image
    image = resImage~getImage(IDB_PROD_ICON)			  -- Create an image from the Product bitmap
    stImage = self~newStatic(IDC_PRODABT_ICON_PLACE)~setImage(image) -- Create a static text control and set the image in it
    font = self~createFontEx("Ariel", 12)			  -- Create up a largish font with which to display text and ...
    self~newStatic(IDC_PRODABT_STATIC2)~setFont(font)		  -- ... set the static text to use that font.
    -- Provide for a double-click in Product icon:
    self~connectStaticNotify("IDC_PRODABT_ICON_PLACE", "DBLCLK", showMsgBox)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method showMsgBox
    say "AboutDialog-showMsgBox-01."
    ans = MessageDialog(.HRS~AboutDblClick)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method leaving
    expose font image
    self~deleteFont(font)
    image~release()
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Human-Readable Strings (HRS)					  v00-02 08Aug11
  --------
   The HRS class provides constant character strings for user-visible messages.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT AboutDblClick  "You double-clicked!"
  ::CONSTANT badRatio	    "The new price/UOM ratio cannot be changed more than 50% up or down."
  ::CONSTANT nilSaved       "Nothing was changed! Data not saved."
  ::CONSTANT saved	    "Changes saved."
  ::CONSTANT uomTooSmall    "The new UOM is too small."
  ::CONSTANT uomTooBig      "The new UOM is too large."
  ::CONSTANT notSaved       "Changes Not Saved!"
  ::CONSTANT descrTooBig    "The Product Description is too long."
  ::CONSTANT prodNameTooBig "The Product Name is too long."
  ::CONSTANT updateProd     "Update Product"
