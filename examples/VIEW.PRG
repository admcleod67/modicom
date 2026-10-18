#include "inkey.ch"
#include "setcurs.ch"



static nRow := 0, nCol := 0, nLineNo := 1
static cCatNo := "", mText := ""
static nLines := 0


procedure view
local nKey := 0, nRefNo :=0

use items
if file("catno.ntx")
   set index to catno
endif
setcolor("W+/B")
clear

mText := items->text
nLines := mlcount(mText, 80)
showpage(nRow, nLineNo, mText, nLines)
setpos(nRow, nCol)

do while nKey != K_ESC

   nKey := 0
   do while nKey == 0
      nKey := inkey()
   enddo

   do case

      case nKey == K_ALT_P
         printpage()

      case nKey == K_ALT_G
         gotoitem()

      case nKey == K_UP
         upline()

      case nKey == K_DOWN
         downline()

      case nKey == K_LEFT
         nCol := IIF(nCol > 0, nCol - 1, 0)
         setpos(nRow, nCol)

      case nKey == K_RIGHT
         nCol := IIF(nCol < maxcol(), nCol + 1, maxcol())
         setpos(nRow, nCol)

      case nKey == K_PGUP
         for i := 1 to 24
            upline()
         next i

      case nKey == K_PGDN
         for i := 1 to 24
            downline()
         next i

      case nKey == K_CTRL_PGUP
         nRow := 0
         nCol := 0
         nLineNo := 1
         goto top
         mText := items->text
         nLines := mlcount(mText, 80)
         showpage(nRow, nLineNo, mText, nLines)
         setpos(nRow, nCol)

      case nKey == K_CTRL_PGDN
         nRow := 0
         nCol := 0
         goto bottom
         mText := items->text
         nLines := mlcount(mText, 80)
         nLineNo := nLines
         clear
         showpage(nRow, nLineNo, mText, nLines)
         setpos(nRow, nCol)

      case nKey == K_RETURN .or. nKey == K_F2 .or. nKey == K_F3
         if nKey == K_F3
            skip
         endif
         if nKey != K_RETURN
            nRow := 0
            nCol := 0
            nLineNo := 1
            nRefNo := newref()
            append blank
            replace refno with nRefNo
            mText := ""
            clear
         endif
         mText := pe(mText)
         nLines := mlcount(mText, 80)
         replace text with mText
         cCatNo := findcatno(memoline(mText, 80, 1))
         replace catno with expand(cCatNo)
         if nLineNo > nLines
            nRow := nRow - (nLineNo - nLines)
            nRow := iif(nRow < 0, 0, nRow)
            nLineNo := nLines
         endif
         if nRow+1 < nLineNo
            showpage(0, nLineNo - nRow, mText, nLines)
         else
            showpage(nRow+1 - nLineNo, 1, mText, nLines)
         endif
         setpos(nRow, nCol)

      case nKey == K_F5
         goto top
         do while .not. eof()
            mText := items->text
            cCatNo := findcatno(memoline(mText, 80, 1))
            replace catno with expand(cCatNo)
            skip
         enddo
         index on catno to catno
         nRow := 0
         nCol := 0
         nLineNo := 1
         goto top
         mText := items->text
         nLines := mlcount(mText, 80)
         showpage(nRow, nLineNo, mText, nLines)
         setpos(nRow, nCol)

      case nKey == K_F6
         set order to 0
         goto top
         mText := items->text
         nLines := mlcount(mText, 80)
         showpage(nRow, nLineNo, mText, nLines)
         setpos(nRow, nCol)

      case nKey == K_F7
         set order to 1
         goto top
         mText := items->text
         nLines := mlcount(mText, 80)
         showpage(nRow, nLineNo, mText, nLines)
         setpos(nRow, nCol)

   endcase

enddo

close all
setcolor("W/N")
clear

return



procedure showpage (nRow, nLineNo, mText, nLines)
local nRecNo := recno()
setpos(nRow, 0)
do while nRow < 25 .and. .not. eof()
   @ nRow, 0 say memoline(mText, 80, nLineNo)
   if nLineNo > nLines
      skip
      if eof()
         skip -1
         return
      endif
      nLineNo := 0
      mText := items->text
      nLines := mlcount(mText, 80)
   endif
   nRow ++
   nLineNo ++
enddo
if nRecNo != 0
   goto nRecNo
endif
return



procedure printpage
local i := 0
set console off
set printer on
for i := 1 to nLines
   ? memoline(mText, 80, i)
next i
eject
set printer off
set console on
return



procedure upline
if nLineNo < 2
   skip -1
   if bof()
      return
   endif
   mText := items->text
   nLines := mlcount(mText, 80)
   nLineNo := nLines + 2
endif
if nRow == 0
   scroll(0, 0, maxrow(), maxcol(), -1)
else
   nRow --
endif
nLineNo --
setcursor(SC_NONE)
@ nRow, 0 say memoline(mText, 80, nLineNo)
setpos(nRow, nCol)
setcursor(SC_NORMAL)
return



procedure downline
if nLineNo > nLines
   skip
   if eof()
      skip -1
      return
   endif
   nLineNo := 0
   mText := items->text
   nLines := mlcount(mText, 80)
endif
if nRow == maxrow()
   scroll(0, 0, maxrow(), maxcol(), 1)
else
   nRow ++
endif
nLineNo ++
setcursor(SC_NONE)
@ nRow, 0 say memoline(mText, 80, nLineNo)
setpos(nRow, nCol)
setcursor(SC_NORMAL)
return



procedure gotoitem
local nOldRecNo := recno(), pattern := ""

    pattern := GetPrompt("Search for:", PadR(pattern, 64))
    pattern := Ltrim(Rtrim(pattern))
    if ( !Empty(pattern) )
        seek expand(pattern)
        if found()
           mText := items->text
           nLines := mlcount(mText, 80)
           nRow := 0
           nLineNo := 1
           showpage(nRow, nLineNo, mText, nLines)
           setpos(nRow, nCol)
        else
           goto nOldRecNo
        endif
    else
        @ 24,0 clear
    end

return (NIL)



function editmemo( cString )
local cScreen := savescreen(0, 0, maxrow(), maxcol())
@ 0, 0 clear to maxrow(), maxcol()
cString := memoedit(cString, 0, 0, maxrow(), maxcol())
restscreen(0, 0, maxrow(), maxcol(), cScreen)
return cString

        

function newref()
local nFirst := 0, nLast := 1000, nRefNo := 0
if eof() .and. .not. bof()
   skip -1
   nRefNo := items->refno
else
   nLast := items->refno
   skip -1
   if .not. bof()
      nFirst := items->refno
   endif
   nRefNo := (nLast - nFirst) / 2 + nFirst
endif
return nRefNo


****
*   Prompt()
*

func GetPrompt(cSay, cGet)
local getList := {}, bInsSave, bAltISave

local nStatCol := MaxCol() - 19


    bInsSave := SetKey(K_INS, {|| SetCursor(if( Set(_SET_INSERT,            ;
                                                !Set(_SET_INSERT) ),        ;
                                                SC_NORMAL, SC_INSERT) ) }   ;
                      )

    bAltISave := SetKey(K_ALT_I, SetKey(K_INS))

    @ 24,0 CLEAR
    @ 24,0 SAY cSay    ;
                GET cGet    ;
                Picture "@KS" + Ltrim(Str(nStatCol - (Len(cSay) + 2)))
    READ

    SetKey(K_INS, bInsSave)
    SetKey(K_ALT_I, bAltISave)

return (cGet)
