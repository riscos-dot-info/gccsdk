{
A unit to watch the heap, i.e. check if all pointers that were
allocated are released again. This is meant as a debugging help
for avoiding memory leaks.

Use it in the main program before all other units. When, at the
end of the program, some pointers that were allocated, have not
been released, the unit prints a message to StdErr. Only pointers
allocated via the Pascal mechanisms (New, GetMem) are tracked, not
pointers allocated with direct libc calls or from C code. After a
runtime error, pointers are not checked.

Note that many units and libraries allocate memory for their own
purposes and don't always release it at the end. Therefore, the
usefulness of this unit is rather limited.

Copyright (C) 1998-2000 Free Software Foundation, Inc.

Author: Frank Heckenbach <frank@pascal.gnu.de>

This file is part of GNU Pascal.

GNU Pascal is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Pascal is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Pascal; see the file COPYING. If not, write to the
Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.

As a special exception, if you link this file with files compiled
with a GNU compiler to produce an executable, this does not cause
the resulting executable to be covered by the GNU General Public
License. This exception does not however invalidate any other
reasons why the executable file might be covered by the GNU General
Public License.
}

{$gnu-pascal}

unit HeapMon;

interface

uses GPC;

{ This unit has an empty interface. It is automatically activated
  when used. }

implementation

var
  HeapMonitorMark : Pointer = nil;

to begin do
  Mark (HeapMonitorMark);

to end do
  if (HeapMonitorMark <> nil) and (ErrorAddr = nil) then
    begin
      var Count : SizeType = ReleaseCount (HeapMonitorMark);
      if Count <> 0 then
        begin
          RestoreTerminal (True);
          Writeln (StdErr, 'Heap monitor: ', Count, ' pointer(s) were not disposed.');
          Halt (7)
        end
    end;

end.
