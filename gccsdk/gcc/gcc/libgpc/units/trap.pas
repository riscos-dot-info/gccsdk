{
Trapping runtime errors

The Trap unit allows you to trap runtime errors, so a runtime error
will not abort the program, but pass the control back to a point
within the program.

The usage is simple. The TrapExec procedure can be called with a
function (p) as an argument. p must take a boolean argument. p will
immediately be called with False given as its argument. When a
runtime error would otherwise be caused while p is active, p will
instead be called again with True as its argument. After p returns,
runtime errors will not be trapped.

When the program terminates (e.g. by reaching its end or by a Halt
statement) and a runtime error was trapped during the run, Trap will
set the ExitCode and ErrorAddr variables to indicate the trapped
error.

Notes:

- After trapping a runtime error, your program might not be in a
  stable state. If the runtime error was a "minor" one (such as a
  range checking or arithmetic error), it should not be a problem.
  But if you e.g. write a larger application and use Trap to prevent
  a sudden abort caused by an unexpected runtime error, you should
  make the program terminate regularly as soon as possible after a
  trapped error (maybe telling the user to save the data, then
  terminate the program and report the error to you).

- Since the trapping mechanism *jumps* back, it has all the negative
  effects that a (non-local!) `goto' can have! You should be aware
  of the consequences of all active procedures being terminated at
  an arbitrary point!

Copyright (C) 1996-2000 Free Software Foundation, Inc.

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

unit Trap;

interface

uses GPC;

var
  TrappedExitCode : Integer = 0;
  TrappedErrorAddr : Pointer = nil;
  TrappedErrorMessageString : TString = '';

procedure TrapExec (procedure p (Trapped : Boolean));

implementation

{$L trapc.c}
procedure DoSetJmp (procedure p (Trapped : Boolean)); C;
procedure DoLongJmp; C;

var
  TrapFlag : Boolean = False;

procedure TrapExit;
begin
  if ErrorAddr <> nil then
    if TrapFlag then
      begin
        TrappedExitCode := ExitCode;
        TrappedErrorAddr := ErrorAddr;
        TrappedErrorMessageString := ErrorMessageString;
        ErrorAddr := nil;
        ExitCode := 0;
        ErrorMessageString := '';
        DoLongJmp
      end
    else
  else
    if TrappedErrorAddr <> nil then
      begin
        ExitCode := TrappedExitCode;
        ErrorAddr := TrappedErrorAddr;
        ErrorMessageString := TrappedErrorMessageString;
        TrappedErrorAddr := nil
      end
end;

procedure TrapExec (procedure p (Trapped : Boolean));

  procedure DoCall (Trapped : Boolean);
  begin
    AtExit (TrapExit);
    p (Trapped)
  end;

begin
  TrapFlag := True;
  DoSetJmp (DoCall);
  TrapFlag := False;
  AtExit (TrapExit)
end;

end.
