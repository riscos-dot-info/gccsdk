{
Pascal declarations of the GPC RTS that are visible to each program.

This unit contains Pascal declarations of many RTS routines which
are not built into the compiler and can be called from programs.
Don't copy the declarations from this unit into your programs, but
rather include this unit with a `uses' statement. The reason is that
the internal declarations, e.g. the `asmnames', may change, and this
unit will be changed accordingly. @@In the future, this unit might
be included into every program automatically, so there will be no
need for a `uses' statement to make the declarations here available.

Note about `protected var' parameters:
Since const parameters in GPC may be passed by value *or* by
reference internally, possibly depending on the system, `const foo*'
parameters to C functions *cannot* reliably declared as `const' in
Pascal. However, Extended Pascal's `protected var' can be used since
this guarantees passing by reference.

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

module GPC interface;

export
  GPC = all;
  GPC_SP = (eread (*@@not really, but an empty export doesn't work*));
  GPC_EP = (eread (*@@not really, but an empty export doesn't work*));
  GPC_BP = (MaxLongInt, ExitCode, ErrorAddr, Pos);
  GPC_Delphi = (MaxLongInt, Int64, ExitCode, ErrorAddr, Pos,
                SetString, StringOfChar, TextFile, AssignFile,
                CloseFile);

const
  MaxLongInt = High (LongInt);

  { Maximum size of a variable }
  MaxVarSize = MaxInt;

type
  Int64 = Integer (64);

{ ====================== MEMORY MANAGEMENT ======================= }

{ Heap manipulation, from heap.c }

{ GPC implements both Mark/Release and Dispose. Both can be mixed freely
  in the same program. Dispose should be preferred, since it's faster. }

{ C heap management routines. NOTE: if Release is used anywhere in
  the program, CFreeMem and CReAllocMem may not be used for pointers
  that were not allocated with CGetMem. }
function  CGetMem     (Size : SizeType) : Pointer;                        asmname 'malloc';
procedure CFreeMem    (aPointer : Pointer);                               asmname 'free';
function  CReAllocMem (aPointer : Pointer; NewSize : SizeType) : Pointer; asmname 'realloc';

type
  GetMemType     = ^function (Size : SizeType) : Pointer;
  FreeMemType    = ^procedure (aPointer : Pointer);
  ReAllocMemType = ^function (aPointer : Pointer; NewSize : SizeType) : Pointer;

{ These variables can be set to user-defined routines for memory
  allocation/deallocation. GetMemPtr may return nil when
  insufficient memory is available. GetMem/New will produce a
  runtime error then. }
var
  GetMemPtr     : GetMemType; asmname '_p_getmem_ptr'; external;
  FreeMemPtr    : FreeMemType; asmname '_p_freemem_ptr'; external;
  ReAllocMemPtr : ReAllocMemType; asmname '_p_reallocmem_ptr'; external;

  { Points to the lowest byte of heap used }
  HeapBegin : Pointer; asmname '_p_heap_begin'; external;

  { Points to the highest byte of heap used }
  HeapHigh  : Pointer; asmname '_p_heap_high'; external;

const
  UndocumentedReturnNil = Pointer (- 1);

procedure ReAllocMem (var aPointer : Pointer; NewSize : SizeType); asmname '_p_reallocmem';

{ Returns the number of pointers that would be released. aPointer must
  have been marked with Mark. For an example of its usage, see the
  HeapMon unit. }
function  ReleaseCount (aPointer : Pointer) : Integer; asmname '_p_releasecount';

{ Routines to handle endianness, from endian.pas }

{ Boolean constants about endianness and alignment }

const
  BitsBigEndian  = {$ifdef __BITS_LITTLE_ENDIAN__} False
                   {$else}{$ifdef __BITS_BIG_ENDIAN__} True
                   {$else}{$error Bit endianness is not defined!}
                   {$endif}{$endif};

  BytesBigEndian = {$ifdef __BYTES_LITTLE_ENDIAN__} False
                   {$else}{$ifdef __BYTES_BIG_ENDIAN__} True
                   {$else}{$error Byte endianness is not defined!}
                   {$endif}{$endif};

  WordsBigEndian = {$ifdef __WORDS_LITTLE_ENDIAN__} False
                   {$else}{$ifdef __WORDS_BIG_ENDIAN__} True
                   {$else}{$error Word endianness is not defined!}
                   {$endif}{$endif};

  NeedAlignment  = {$ifdef __NEED_ALIGNMENT__} True
                   {$else} False {$endif};

{ Convert single variables from or to little or big endian format.
  This only works for a single variable or a plain array of a simple
  type. For more complicated structures, this has to be done for
  each component separately! Currently, ConvertFromFooEndian and
  ConvertToFooEndian are the same, but this might not be the case on
  middle-endian machines. Therefore, we provide different names. }
procedure ReverseBytes            (var Buf; ElementSize, Count : SizeType); asmname '_p_reversebytes';
procedure ConvertFromLittleEndian (var Buf; ElementSize, Count : SizeType); asmname '_p_convertlittleendian';
procedure ConvertFromBigEndian    (var Buf; ElementSize, Count : SizeType); asmname '_p_convertbigendian';
procedure ConvertToLittleEndian   (var Buf; ElementSize, Count : SizeType); asmname '_p_convertlittleendian';
procedure ConvertToBigEndian      (var Buf; ElementSize, Count : SizeType); asmname '_p_convertbigendian';

{ Read a block from a file and convert it from little or
  big endian format. This only works for a single variable or a
  plain array of a simple type, note the comment for
  `ConvertFromLittleEndian' and `ConvertFromBigEndian'. }
(*@@iocritical*)procedure BlockReadLittleEndian   (var aFile : File; var   Buf; ElementSize, Count : SizeType); asmname '_p_blockread_littleendian';
(*@@iocritical*)procedure BlockReadBigEndian      (var aFile : File; var   Buf; ElementSize, Count : SizeType); asmname '_p_blockread_bigendian';

{ Write a block variable to a file and convert it to little or big
  endian format before. This only works for a single variable or a
  plain array of a simple type. Apart from this, note the comment
  for `ConvertToLittleEndian' and `ConvertToBigEndian'. }
(*@@iocritical*)procedure BlockWriteLittleEndian  (var aFile : File; const Buf; ElementSize, Count : SizeType); asmname '_p_blockwrite_littleendian';
(*@@iocritical*)procedure BlockWriteBigEndian     (var aFile : File; const Buf; ElementSize, Count : SizeType); asmname '_p_blockwrite_bigendian';

{ Read and write strings from/to binary files, where the length is
  stored in the given endianness and with a fixed size (64 bits),
  and therefore is independent of the system. }
(*@@iocritical*)procedure ReadStringLittleEndian  (var f : File; var s : String);   asmname '_p_ReadStringLittleEndian';
(*@@iocritical*)procedure ReadStringBigEndian     (var f : File; var s : String);   asmname '_p_ReadStringBigEndian';
(*@@iocritical*)procedure WriteStringLittleEndian (var f : File; const s : String); asmname '_p_WriteStringLittleEndian';
(*@@iocritical*)procedure WriteStringBigEndian    (var f : File; const s : String); asmname '_p_WriteStringBigEndian';

{ =================== STRING HANDLING ROUTINES =================== }

{ String handling routines, from string.pas }

type
  AnyFile = Text; (*@@ create `AnyFile' parameters*)
  PAnyFile = ^AnyFile;

{ TString is a string type that is used for function results and
  local variables, as long as undiscriminated strings are not
  allowed there. The default size of 2048 characters should be
  enough for file names on any system, but can be changed when
  necessary. It should be at least as big as MAXPATHLEN. }

const
  TStringSize = 2048;
  SpaceCharacters = [' ', #9];
  NewLine = "\n"; { the separator of lines within a string }
  LineBreak = {$ifdef __OS_DOS__} "\r\n" {$else} "\n" {$endif}; { the separator of lines within a file }

type
  TString    = String (TStringSize);
  TStringBuf = packed array [0 .. TStringSize] of Char;
  PString    = ^String;
  CharSet    = set of Char;
  PCStrings  = ^TCStrings;
  TCStrings  = array [0 .. MaxVarSize div SizeOf (CString)] of CString;

var
  CParamCount : Integer; asmname '_p_argc'; external;
  CParameters : PCStrings; asmname '_p_argv'; external;

function  MemCmp      (const s1, s2; Size : SizeType) : Integer; asmname 'memcmp';
function  MemComp     (const s1, s2; Size : SizeType) : Integer; asmname 'memcmp';
function  MemCompCase (const s1, s2; Size : SizeType) : Boolean; asmname '_p_memcmpcase';

procedure UpCaseString    (var s : String);                                      asmname '_p_upcase_string';
procedure LoCaseString    (var s : String);                                      asmname '_p_locase_string';
function  UpCaseStr       (const s : String) : TString;                          asmname '_p_upcase_str';
function  LoCaseStr       (const s : String) : TString;                          asmname '_p_locase_str';

function  IsUpCase        (ch : Char) : Boolean;                                 attribute (const); asmname '_p_isupcase';
function  IsLoCase        (ch : Char) : Boolean;                                 attribute (const); asmname '_p_islocase';
function  IsAlphaNum      (ch : Char) : Boolean;                                 attribute (const); asmname '_p_isalphanum';
function  IsAlphaNumUnderscore (ch : Char) : Boolean;                            attribute (const); asmname '_p_isalphanumunderscore';
function  IsPrintable     (ch : Char) : Boolean;                                 attribute (const); asmname '_p_isprintable';

function  StrEqualCase    (const s1, s2 : String) : Boolean;                     asmname '_p_strequalcase';

function  Pos             (const SubString, aString : String) : Integer;             asmname '_p_pos';
function  LastPos         (const SubString, aString : String) : Integer;             asmname '_p_lastpos';
function  PosCase         (const SubString, aString : String) : Integer;             asmname '_p_poscase';
function  LastPosCase     (const SubString, aString : String) : Integer;             asmname '_p_lastposcase';
function  CharPos         (const Chars : CharSet; const aString : String) : Integer; asmname '_p_charpos';
function  LastCharPos     (const Chars : CharSet; const aString : String) : Integer; asmname '_p_lastcharpos';

function  PosFrom         (const SubString, aString : String; From : Integer) : Integer;             asmname '_p_posfrom';
function  LastPosTill     (const SubString, aString : String; Till : Integer) : Integer;             asmname '_p_lastpostill';
function  PosFromCase     (const SubString, aString : String; From : Integer) : Integer;             asmname '_p_posfromcase';
function  LastPosTillCase (const SubString, aString : String; Till : Integer) : Integer;             asmname '_p_lastpostillcase';
function  CharPosFrom     (const Chars : CharSet; const aString : String; From : Integer) : Integer; asmname '_p_charposfrom';
function  LastCharPosTill (const Chars : CharSet; const aString : String; Till : Integer) : Integer; asmname '_p_lastcharpostill';

function  IsPrefix        (const Prefix, s : String) : Boolean;                     asmname '_p_isprefix';
function  IsSuffix        (const Suffix, s : String) : Boolean;                     asmname '_p_issuffix';
function  IsPrefixCase    (const Prefix, s : String) : Boolean;                     asmname '_p_isprefixcase';
function  IsSuffixCase    (const Suffix, s : String) : Boolean;                     asmname '_p_issuffixcase';

function  CStringLength      (Src : CString) : SizeType;                            asmname '_p_strlen';
function  CStringEnd         (Src : CString) : CString;                             asmname '_p_strend';
function  CStringNew         (Src : CString) : CString;                             asmname '_p_strdup';
function  CStringComp        (s1, s2 : CString) : Integer;                          asmname '_p_strcmp';
function  CStringCaseComp    (s1, s2 : CString) : Integer;                          asmname '_p_strcasecmp';
function  CStringLComp       (s1, s2 : CString; MaxLen : SizeType) : Integer;       asmname '_p_strlcmp';
function  CStringLCaseComp   (s1, s2 : CString; MaxLen : SizeType) : Integer;       asmname '_p_strlcasecmp';
function  CStringCopy        (Dest, Source : CString) : CString;                    asmname '_p_strcpy';
function  CStringCopyEnd     (Dest, Source : CString) : CString;                    asmname '_p_strecpy';
function  CStringLCopy       (Dest, Source : CString; MaxLen : SizeType) : CString; asmname '_p_strlcpy';
function  CStringMove        (Dest, Source : CString; Count : SizeType) : CString;  asmname '_p_strmove';
function  CStringCat         (Dest, Source : CString) : CString;                    asmname '_p_strcat';
function  CStringLCat        (Dest, Source : CString; MaxLen : SizeType) : CString; asmname '_p_strlcat';
function  CStringCharPos     (Src : CString; Ch : Char) : CString;                  asmname '_p_strscan';
function  CStringLastCharPos (Src : CString; Ch : Char) : CString;                  asmname '_p_strrscan';
function  CStringPos         (aString, SubString : CString) : CString;              asmname '_p_strpos';
function  CStringLastPos     (aString, SubString : CString) : CString;              asmname '_p_strrpos';
function  CStringCasePos     (aString, SubString : CString) : CString;              asmname '_p_strcasepos';
function  CStringLastCasePos (aString, SubString : CString) : CString;              asmname '_p_strrcasepos';
function  CStringUpCase      (s : CString) : CString;                               asmname '_p_strupper';
function  CStringLoCase      (s : CString) : CString;                               asmname '_p_strlower';
function  CStringIsEmpty     (s : CString) : Boolean;                               asmname '_p_strempty';
function  NewCString         (const Source : String) : CString;                     asmname '_p_newcstring';
function  CStringCopyString  (Dest : CString; const Source : String) : CString;     asmname '_p_cstringcopystring';
procedure CopyCString        (Source : CString; var Dest : String);                 asmname '_p_copycstring';

function  NewString       (const s : String) : PString;                          asmname '_p_newstring';
procedure DisposeString   (p : PString);                                         asmname '_p_dispose';

procedure SetString       (var s : String; Buffer : PChar; Count : Integer);     asmname '_p_set_string';
function  StringOfChar    (Ch : Char; Count : Integer) = s : TString;            asmname '_p_string_of_char';

procedure TrimLeft        (var s : String);                                      asmname '_p_trimleft';
procedure TrimRight       (var s : String);                                      asmname '_p_trimright';
procedure TrimBoth        (var s : String);                                      asmname '_p_trimboth';
function  TrimLeftStr     (const s : String) : TString;                          asmname '_p_trimleft_str';
function  TrimRightStr    (const s : String) : TString;                          asmname '_p_trimright_str';
function  TrimBothStr     (const s : String) : TString;                          asmname '_p_trimboth_str';

function  GetStringCapacity (const s : String) : Integer;                        asmname '_p_get_string_capacity';

{ A short cut for a common use of WriteStr as a function }
function  Integer2String (i : Integer) : TString;                                asmname '_p_Integer2String';

type
  TChars = packed array [1 .. 1] of Char;
  PChars = ^TChars;

  { Under development. Interface subject to change.
    Use with caution. }
  { When a const or var AnyString parameter is passed, internally
    these records are passed as const parameters. Value AnyString
    parameters are passed like value string parameters. }
  ConstAnyString = record
    Length : Integer;
    Chars  : PChars
  end;

  { Capacity is the allocated space (used internally). Count is the
    actual number of environment strings. The CStrings array
    contains the environment strings, terminated by a nil pointer,
    which is not counted in Count. @CStrings can be passed to libc
    routines like execve which expect an environment (see
    GetCEnvironment). }
  PEnvironment = ^TEnvironment;
  TEnvironment (Capacity : Integer) = record
    Count : Integer;
    CStrings : array [1 .. Capacity + 1] of CString
  end;

var
  Environment : PEnvironment; asmname '_p_environment'; external;

{ Get an environment variable. If it does not exist, GetEnv returns
  the empty string, which can't be distinguished from a variable
  with an empty value, while CStringGetEnv returns nil then. Note,
  Dos doesn't know empty environment variables, but treats them as
  non-existing, and does not distinguish case in the names of
  environment variables. However, even under Dos, empty environment
  variables and variable names with different case can now be set
  and used within GPC programs. }
function  GetEnv (const EnvVar : String) : TString;                        asmname '_p_getenv';
function  CStringGetEnv (EnvVar : CString) : CString;                      asmname '_p_cstringgetenv';

{ Sets an environment variable with the name given in VarName to the value
  Value. A previous value, if any, is overwritten. }
procedure SetEnv (const VarName, Value : String);                          asmname '_p_setenv';

{ Un-sets an environment variable with the name given in VarName. }
procedure UnSetEnv (const VarName : String);                               asmname '_p_unsetenv';

{ Returns @Environment^.CStrings, converted to PCStrings, to be passed to
  libc routines like execve which expect an environment. }
function  GetCEnvironment : PCStrings;                                     asmname '_p_getcenvironment';

{ ================= RUNTIME ERROR HANDLING ETC. ================== }

{ Error handling functions, from error.pas }

const
  ERead = 413;
  EWrite = 414;
  EWriteReadOnly = 422;

var
  { Error number (after runtime error) or exit status (after Halt) or
    0 (during program run and after succesful termination). }
  ExitCode : Integer; asmname '_p_exitcode'; external;

  { Contains the address of the code where a runtime occurred, nil
    if no runtime error occurred. }
  ErrorAddr : Pointer; asmname '_p_erroraddr'; external;

  { Error message }
  ErrorMessageString : TString; asmname '_p_errormessagestring'; external;

function  GetErrorMessage                 (n : Integer) : CString;                   asmname '_p_errmsg';
procedure RuntimeError                    (n : Integer);                             attribute (noreturn); asmname '_p_error';
procedure RuntimeErrorInteger             (n : Integer; i : MedInt);                 attribute (noreturn); asmname '_p_error_integer';
procedure RuntimeErrorCString             (n : Integer; s : CString);                attribute (noreturn); asmname '_p_error_string';
procedure InternalError                   (n : Integer);                             attribute (noreturn); asmname '_p_internal_error';
procedure InternalErrorInteger            (n : Integer; i : MedInt);                 attribute (noreturn); asmname '_p_internal_error_integer';
procedure RuntimeWarning                  (Message : CString);                       asmname '_p_warning';
procedure RuntimeWarningInteger           (Message : CString; i : MedInt);           asmname '_p_warning_integer';
procedure RuntimeWarningCString           (Message : CString; s : CString);          asmname '_p_warning_string';
procedure DebugStatement                  (const FileName : String; Line : Integer); asmname '_p_debug_statement';

(*iocritical*)procedure IOError                         (n : Integer);                             asmname '_p_io_error';
(*iocritical*)procedure IOErrorInteger                  (n : Integer; i : MedInt);                 asmname '_p_io_error_integer';
(*iocritical*)procedure IOErrorCString                  (n : Integer; s : CString);                asmname '_p_io_error_string';
(*iocritical*)procedure IOErrorFile                     (n : Integer; protected var f : AnyFile);  asmname '_p_io_error_file';
function  GetIOErrorMessage : CString;                                               asmname '_p_get_io_error_message';
procedure CheckInOutRes;                                                             asmname '_p_check_inoutres';

{ Registers a procedure to be called to restore the terminal for
  another process that accesses the terminal, or back for the
  program itself. Used e.g. by the CRT unit. The procedures must
  allow for being called multiple times in any order, even at the
  end of the program (see the comment for RestoreTerminal). }
procedure RegisterRestoreTerminal (ForAnotherProcess : Boolean; procedure Proc); asmname '_p_RegisterRestoreTerminal';

{ Unregisters a procedure registered with RegisterRestoreTerminal.
  Returns False if the procedure had not been registered, and True
  if it had been registered and was unregistered successfully. }
function UnregisterRestoreTerminal (ForAnotherProcess : Boolean; procedure Proc) : Boolean; asmname '_p_UnregisterRestoreTerminal';

{ Calls the procedures registered by RegisterRestoreTerminal. When
  restoring the terminal for another process, the procedures are
  called in the opposite order of registration. When restoring back
  for the program, they are called in the order of registration.

  `RestoreTerminal (True)' will also be called at the end of the
  program, before outputting any runtime error message. It can also
  be used if you want to write an error message and exit the program
  (especially when using e.g. the CRT unit). For this purpose, to
  avoid side effects, call RestoreTerminal immediately before
  writing the error message (to StdErr, not to Output!), and then
  exit the program (e.g. with Halt). }
procedure RestoreTerminal (ForAnotherProcess : Boolean); asmname '_p_RestoreTerminal';

{ Executes a command line. Reports execution errors via the IOResult
  mechanism and returns the exit status of the executed program.
  Execute calls RestoreTerminal with the argument True before and
  False after executing the process, ExecuteNoTerminal does not. }
(*@@IO critical*)function  Execute (const CmdLine : String) : Integer; asmname '_p_execute';
(*@@IO critical*)function  ExecuteNoTerminal (const CmdLine : String) : Integer; asmname '_p_executenoterminal';

procedure AtExit (procedure Proc); asmname '_p_atexit';

procedure SetReturnAddress (Address : Pointer);                                      asmname '_p_SetReturnAddress';
procedure RestoreReturnAddress;                                                      asmname '_p_RestoreReturnAddress';

{ ==================== SIGNALS AND PROCESSES ===================== }

function ProcessID : Integer; asmname '_p_pid';

{ Extract information from the status returned by PWait }
function StatusExited     (Status : Integer) : Boolean; attribute (const); asmname '_p_WIfExited';
function StatusExitCode   (Status : Integer) : Integer; attribute (const); asmname '_p_WExitStatus';
function StatusSignaled   (Status : Integer) : Boolean; attribute (const); asmname '_p_WIfSignaled';
function StatusTermSignal (Status : Integer) : Integer; attribute (const); asmname '_p_WTermSig';
function StatusStopped    (Status : Integer) : Boolean; attribute (const); asmname '_p_WIfStopped';
function StatusStopSignal (Status : Integer) : Integer; attribute (const); asmname '_p_WStopSig';

type
  TSignalHandler = procedure (Signal : Integer);

{ OldHandler and OldRestart may be null }
function InstallSignalHandler (Signal : Integer; Handler : TSignalHandler;
                               Restart, UnlessIgnored : Boolean;
                               var OldHandler : TSignalHandler; var OldRestart : Boolean) : Boolean; asmname '_p_sigaction';

var
  { Signal actions }
  SignalDefault : TSignalHandler; asmname '_p_SIG_DFL'; external;
  SignalIgnore  : TSignalHandler; asmname '_p_SIG_IGN'; external;
  SignalError   : TSignalHandler; asmname '_p_SIG_ERR'; external;

  { Signals. The constants are set to the signal numbers, and
    are 0 for signals not defined. }
  { POSIX signals }
  SigHUp    : Integer; asmname '_p_SIGHUP'; external;
  SigInt    : Integer; asmname '_p_SIGINT'; external;
  SigQuit   : Integer; asmname '_p_SIGQUIT'; external;
  SigIll    : Integer; asmname '_p_SIGILL'; external;
  SigAbrt   : Integer; asmname '_p_SIGABRT'; external;
  SigFPE    : Integer; asmname '_p_SIGFPE'; external;
  SigKill   : Integer; asmname '_p_SIGKILL'; external;
  SigSegV   : Integer; asmname '_p_SIGSEGV'; external;
  SigPipe   : Integer; asmname '_p_SIGPIPE'; external;
  SigAlrm   : Integer; asmname '_p_SIGALRM'; external;
  SigTerm   : Integer; asmname '_p_SIGTERM'; external;
  SigUsr1   : Integer; asmname '_p_SIGUSR1'; external;
  SigUsr2   : Integer; asmname '_p_SIGUSR2'; external;
  SigChld   : Integer; asmname '_p_SIGCHLD'; external;
  SigCont   : Integer; asmname '_p_SIGCONT'; external;
  SigStop   : Integer; asmname '_p_SIGSTOP'; external;
  SigTStp   : Integer; asmname '_p_SIGTSTP'; external;
  SigTTIn   : Integer; asmname '_p_SIGTTIN'; external;
  SigTTOu   : Integer; asmname '_p_SIGTTOU'; external;

  { Non-POSIX signals }
  SigTrap   : Integer; asmname '_p_SIGTRAP'; external;
  SigIOT    : Integer; asmname '_p_SIGIOT'; external;
  SigEMT    : Integer; asmname '_p_SIGEMT'; external;
  SigBus    : Integer; asmname '_p_SIGBUS'; external;
  SigSys    : Integer; asmname '_p_SIGSYS'; external;
  SigStkFlt : Integer; asmname '_p_SIGSTKFLT'; external;
  SigUrg    : Integer; asmname '_p_SIGURG'; external;
  SigIO     : Integer; asmname '_p_SIGIO'; external;
  SigPoll   : Integer; asmname '_p_SIGPOLL'; external;
  SigXCPU   : Integer; asmname '_p_SIGXCPU'; external;
  SigXFSz   : Integer; asmname '_p_SIGXFSZ'; external;
  SigVTAlrm : Integer; asmname '_p_SIGVTALRM'; external;
  SigProf   : Integer; asmname '_p_SIGPROF'; external;
  SigPwr    : Integer; asmname '_p_SIGPWR'; external;
  SigInfo   : Integer; asmname '_p_SIGINFO'; external;
  SigLost   : Integer; asmname '_p_SIGLOST'; external;
  SigWinCh  : Integer; asmname '_p_SIGWINCH'; external;

  { Signal subcodes (only used on some systems, -1 if not used) }
  IllReservedAddress        : Integer; asmname '_p_ILL_RESAD_FAULT'; external;
  IllPriviledgedInstruction : Integer; asmname '_p_ILL_PRIVIN_FAULT'; external;
  IllReservedOp             : Integer; asmname '_p_ILL_RESOP_FAULT'; external;
  FPEIntegerOverflow        : Integer; asmname '_p_FPE_INTOVF_TRAP'; external;
  FPEIntegerDivisionByZero  : Integer; asmname '_p_FPE_INTDIV_TRAP'; external;
  FPESubscriptRange         : Integer; asmname '_p_FPE_SUBRNG_TRAP'; external;
  FPERealOverflow           : Integer; asmname '_p_FPE_FLTOVF_TRAP'; external;
  FPERealDivisionByZero     : Integer; asmname '_p_FPE_FLTDIV_TRAP'; external;
  FPERealUnderflow          : Integer; asmname '_p_FPE_FLTUND_TRAP'; external;
  FPEDecimalOverflow        : Integer; asmname '_p_FPE_DECOVF_TRAP'; external;

{ Returns a description for a signal }
function StrSignal (Signal : Integer) : TString; asmname '_p_strsignal';

{ Sends a signal to a process. Returns True if successful. If Signal
  is 0, it doesn't send a signal, but still checks whether it would
  be possible to send a signal to the given process. }
function Kill (PID, Signal : Integer) : Boolean; asmname '_p_kill';

const
  AnyChild = - 1;

{ Waits for a child process with the given PID (or any child process
  if PID = AnyChild) to terminate or be stopped. Returns the PID of
  the process. WStatus will contain the status and can be evaluated
  with StatusExited etc.. If nothing happened, and Block is False,
  the function will return 0, and WStatus will be 0. If an error
  occurred (especially on single tasking systems where WaitPID is
  not possible), the function will return a negative value, and
  WStatus will be 0. }
function WaitPID (PID : Integer; var WStatus : Integer; Block : Boolean) : Integer; asmname '_p_waitpid';

{ Sets the process group of Process (or the current one if Process
  is 0) to ProcessGroup (or its PID if ProcessGroup is 0). Returns
  True if successful. }
function SetProcessGroup (Process, ProcessGroup : Integer) : Boolean; asmname '_p_setpgid';

{ Sets the process group of a terminal given by Terminal (as a file
  handle) to ProcessGroup. ProcessGroup must be the ID of a process
  group in the same session. Returns True if successful. }
function SetTerminalProcessGroup (Terminal, ProcessGroup : Integer) : Boolean; asmname '_p_tcsetpgrp';

{ Returns the process group of a terminal given by Terminal (as a
  file handle), or -1 on error. }
function GetTerminalProcessGroup (Terminal : Integer) : Integer; asmname '_p_tcgetpgrp';

{ ================= COMMAND LINE OPTION PARSING ================== }

const
  EndOfOptions      = #255;
  NoOption          = #1;
  UnknownOption     = '?';
  LongOption        = #0;
  UnknownLongOption = '?';

var
  FirstNonOption         : Integer; asmname '_p_first_non_option'; external;
  HasOptionArgument      : Boolean; asmname '_p_has_option_argument'; external;
  OptionArgument         : TString; asmname '_p_option_argument'; external;
  UnknownOptionCharacter : Char; asmname '_p_unknown_option_character'; external;
  GetOptErrorFlag        : Boolean; asmname '_p_getopt_error_flag'; external;

{
  Parses command line arguments for options and returns the next
  one.

  If a command line argument starts with `-', and is not exactly `-'
  or `--', then it is an option element. The characters of this
  element (aside from the initial `-') are option characters. If
  `GetOpt' is called repeatedly, it returns successively each of the
  option characters from each of the option elements.

  If `GetOpt' finds another option character, it returns that
  character, updating `FirstNonOption' and internal variables so
  that the next call to `GetOpt' can resume the scan with the
  following option character or command line argument.

  If there are no more option characters, `GetOpt' returns
  EndOfOptions. Then `FirstNonOption' is the index of the first
  command line argument that is not an option. (The command line
  arguments have been permuted so that those that are not options
  now come last.)

  OptString must be of the form `[+|-]abcd:e:f:g::h::i::'.

  a, b, c are options without arguments
  d, e, f are options with required arguments
  g, h, i are options with optional arguments

  Arguments are text following the option character in the same
  command line argument, or the text of the following command line
  argument. They are returned in OptionArgument. If an option has no
  argument, OptionArgument is empty. The variable HasOptionArgument
  tells whether an option has an argument. This is mostly useful for
  options with optional arguments, if one wants to distinguish an
  empty argument from no argument.

  If the first character of OptString is `+', GetOpt stops at the
  first non-option argument.

  If it is `-', GetOpt treats non-option arguments as options and
  return NoOption for them.

  Otherwise, GetOpt permutes arguments and handles all options,
  leaving all non-options at the end. However, if the environment
  variable POSIXLY_CORRECT is set, the default behaviour is to stop
  at the first non-option argument, as with `+'.

  The special argument `--' forces an end of option-scanning
  regardless of the first character of OptString. In the case of
  `-', only `--' can cause GetOpt to return EndOfOptions with
  FirstNonOption <= ParamCount.

  If an option character is seen that is not listed in OptString,
  UnknownOption is returned. The unrecognized option character is
  stored in UnknownOptionCharacter. If you set GetOptErrorFlag to
  True, an error message is printed to StdErr automatically.
}
function GetOpt (OptString : CString) : Char; asmname '_p_getopt';

type
  OptArgType = (NoArgument, RequiredArgument, OptionalArgument);

  OptionType = record
    Name     : CString;
    Argument : OptArgType;
    Flag     : ^Char; { if nil, V is returned. Otherwise, Flag^ is }
    V        : Char   { set to V, and Ord (LongOption) is returned }
  end;

{
  Recognize short options, described by OptString as above, and long
  options, described by LongOptions.

  Long-named options begin with `--' instead of `-'. Their names may
  be abbreviated as long as the abbreviation is unique or is an
  exact match for some defined option. If they have an argument, it
  follows the option name in the same argument, separated from the
  option name by a `=', or else the in next argument. When GetOpt
  finds a long-named option, it returns LongOption if that option's
  `Flag' field is non-nil, and the value of the option's `V' field
  if the `Flag' field is nil.

  LongIndex, if not null, returns the index in LongOptions of the
  long-named option found. It is only valid when a long-named option
  has been found by the most recent call.

  If LongOnly is set, `-' as well as `--' can indicate a long
  option. If an option that starts with `-' (not `--') doesn't match
  a long option, but does match a short option, it is parsed as a
  short option instead. If an argument has the form `-f', where f is
  a valid short option, don't consider it an abbreviated form of a
  long option that starts with `f'. Otherwise there would be no way
  to give the `-f' short option. On the other hand, if there's a
  long option `fubar' and the argument is `-fu', do consider that an
  abbreviation of the long option, just like `--fu', and not `-f'
  with argument `u'. This distinction seems to be the most useful
  approach.
}
function GetOptLong (OptString : CString; var LongOptions : array [m .. n : Integer] of OptionType { can be null };
                     var LongIndex : Integer { can be null }; LongOnly : Boolean) : Char; asmname '_p_getopt_long';

{ =========================== PEXECUTE =========================== }

const
  PExecute_First   = 1;
  PExecute_Last    = 2;
  PExecute_One     = PExecute_First or PExecute_Last;
  PExecute_Search  = 4;
  PExecute_Verbose = 8;

{
  PExecute: execute a program.

  Program and Arguments are the arguments to execv/execvp.

  Flags and PExecute_Search is non-zero if $PATH should be searched
  (It's not clear that GCC passes this flag correctly). Flags and
  PExecute_First is nonzero for the first process in chain. Flags
  and PExecute_Last is nonzero for the last process in chain.

  The result is the pid on systems like Unix where we fork/exec and
  on systems like MS-Windows and OS2 where we use spawn. It is up to
  the caller to wait for the child.

  The result is the exit code on systems like MSDOS where we spawn
  and wait for the child here.

  Upon failure, ErrMsg is set to the text of the error message,
  and -1 is returned. `errno' is available to the caller to use.

  PWait: cover function for wait.

  PID is the process id of the task to wait for. Status is the
  `status' argument to wait. Flags is currently unused (allows
  future enhancement without breaking upward compatibility). Pass 0
  for now.

  The result is the process ID of the child reaped, or -1 for
  failure.

  On systems that don't support waiting for a particular child, PID
  is ignored. On systems like MSDOS that don't really multitask
  PWait is just a mechanism to provide a consistent interface for
  the caller.
}
function PExecute (ProgramName : CString; Arguments : PCStrings; var ErrMsg : String; Flags : Integer) : Integer; asmname '_p_pexecute';
function PWait (PID : Integer; var Status : Integer; Flags : Integer) : Integer; C;

{ ==================== TIME HANDLING ROUTINES ==================== }

{ Time and date routines for Extended Pascal, from time.pas }

const { from types.h }
  DateLength = 11;
  TimeLength = 8;

type
  UnixTimeType = LongInt; { This is hard-coded in the compiler. Do not change here. }
  MicroSecondTimeType = LongInt;

  DateString = packed array [1 .. DateLength] of Char;
  TimeString = packed array [1 .. TimeLength] of Char;

var
  MonthName : array [1 .. 12] of String [9]; asmname '_p_monthname'; external;
  MonthLength : array [1 .. 12] of Integer; asmname '_p_monthlength'; external;

function  GetDayOfWeek (Day, Month, Year : Integer) : Integer;                                            asmname '_p_dayofweek';
procedure UnixTimeToTimeStamp (UnixTime : UnixTimeType; var aTimeStamp : TimeStamp);                      asmname '_p_unix_time_to_time_stamp';
function  TimeStampToUnixTime (protected var aTimeStamp : TimeStamp) : UnixTimeType;                      asmname '_p_time_stamp_to_unix_time';
function  GetMicroSecondTime : MicroSecondTimeType;                                                       asmname '_p_get_micro_second_time';

{ Is the year a leap year? }
function  IsLeapYear (Year : Integer) : Boolean;                                                          asmname '_p_is_leap_year';

procedure Sleep (Seconds : Integer);                                                                      asmname '_p_sleep';
procedure SleepMicroSeconds (MicroSeconds : Integer);                                                     asmname '_p_sleep_microseconds';
procedure UnixTimeToTime (UnixTime : UnixTimeType; var Year, Month, Day, Hour, Minute, Second : Integer); asmname '_p_unix_time_to_time';
function  TimeToUnixTime (Year, Month, Day, Hour, Minute, Second : Integer) : UnixTimeType;               asmname '_p_time_to_unix_time';

{ Get the real time. MicroSecond can be null and is ignored then. }
function  GetUnixTime (var MicroSecond : Integer) : UnixTimeType;                                         asmname '_p_get_unix_time';

{ Get the CPU time used. MicroSecond can be null and is ignored then.
  Now, GetCPUTime can measure long CPU times reliably on most systems
  (e.g. Solaris where it didn't work before). }
function  GetCPUTime (var MicroSecond : Integer) : Integer;                                               asmname '_p_get_cpu_time';

{ ==================== FILE HANDLING ROUTINES ==================== }

{ file routines, from files.pas }

type
  FileSizeType = LongInt;

{ bind.c }

procedure GetBinding   (protected var aFile : AnyFile; var aBinding : BindingType); asmname '_p_binding';
procedure ClearBinding (var aBinding : BindingType);                                asmname '_p_clearbinding';

{ TFDD interface @@ Subject to change! Use with caution! }

type
  TOpenMode   = (foNone, foReset, foRewrite, foAppend, foSeekRead, foSeekWrite, foSeekUpdate);
  TOpenProc   = procedure (var PrivateData; Mode : TOpenMode);
  TSelectFunc = function  (var PrivateData; Writing : Boolean) : Integer; { called before select(), must return a handle }
  TSelectProc = procedure (var PrivateData; var ReadSelect, WriteSelect, ExceptSelect : Boolean); { called before and after select() }
  TReadFunc   = function  (var PrivateData; var   Buffer; Size : SizeType) : SizeType;
  TWriteFunc  = function  (var PrivateData; const Buffer; Size : SizeType) : SizeType;
  TFileProc   = procedure (var PrivateData);
  TFlushProc  = TFileProc;
  TCloseProc  = TFileProc;
  TDoneProc   = TFileProc;

procedure AssignTFDD (var f : AnyFile;
                      OpenProc    : TOpenProc;
                      SelectFunc  : TSelectFunc;
                      SelectProc  : TSelectProc;
                      ReadFunc    : TReadFunc;
                      WriteFunc   : TWriteFunc;
                      FlushProc   : TFlushProc;
                      CloseProc   : TCloseProc;
                      DoneProc    : TDoneProc;
                      PrivateData : Pointer);       asmname '_p_assign_tfdd';

procedure SetTFDD    (var f : AnyFile;
                      OpenProc    : TOpenProc;
                      SelectFunc  : TSelectFunc;
                      SelectProc  : TSelectProc;
                      ReadFunc    : TReadFunc;
                      WriteFunc   : TWriteFunc;
                      FlushProc   : TFlushProc;
                      CloseProc   : TCloseProc;
                      DoneProc    : TDoneProc;
                      PrivateData : Pointer);       asmname '_p_set_tfdd';

{ Any parameter except f may be null }
procedure GetTFDD    (var f : AnyFile;
                      var OpenProc    : TOpenProc;
                      var SelectFunc  : TSelectFunc;
                      var SelectProc  : TSelectProc;
                      var ReadFunc    : TReadFunc;
                      var WriteFunc   : TWriteFunc;
                      var FlushProc   : TFlushProc;
                      var CloseProc   : TCloseProc;
                      var DoneProc    : TDoneProc;
                      var PrivateData : Pointer);   asmname '_p_get_tfdd';

type
  Natural = 1 .. MaxInt;
  IOSelectEvents = (SelectReadOrEOF, SelectRead, SelectEOF, SelectWrite, SelectException, SelectAlways);

const
  IOSelectEventMin = (*Low (IOSelectEvents);*)SelectReadOrEOF;
  IOSelectEventMax = Pred (SelectAlways);

type
  IOSelectType = record
    f : PAnyFile;
    Wanted : set of IOSelectEvents;
    Occurred : set of IOSelectEventMin .. IOSelectEventMax
  end;

{ Waits for one of several events to happen. Returns when one or
  more of the wanted events for one of the files occur. If they have
  already occurred before calling the function, it returns
  immediately. MicroSeconds can specify a timeout. If it is 0, the
  function will return immediately, whether or not an event has
  occurred. If it is negative, the function will wait forever until
  an event occurs. The Events parameter can be null, in which case
  the function only waits for the timeout. If any of the file
  pointers (f) in Events are nil or the files pointed to are closed,
  they are simply ignored for convenience.

  It returns the index of one of the files for which any event has
  occurred. If events have occurred for several files, is it
  undefined which of these file's index is returned. If no event
  occurs until the timeout, 0 is returned. If an error occurs or the
  target system does not have a select() system call and Events is
  not null, a negative value is returned. In the Occurred field of
  the elements of Events, events that have occurred are set. The
  state of events not wanted is undefined.

  The possible events are:
  SelectReadOrEOF: the file is at EOF or data can be read now.
  SelectRead:      data can be read now.
  SelectEOF:       the file is at EOF.
  SelectWrite:     data can be written now.
  SelectException: an exception occurred on the file.
  SelectAlways:    if this is set, *all* requested events will be
                   checked for this file in any case. Otherwise,
                   checks may be skipped if already another event
                   for this or another file was found.

  Notes:
  Checking for EOF requires some reading ahead internally (just like
  the EOF function) which can be avoided by setting SelectReadOrEOF
  instead of SelectRead and SelectEOF. If this is followed by, e.g.,
  a BlockRead with 4 parameters, the last parameter will be 0 if and
  only the file is at EOF, and otherwise, data will be read directly
  from the file without reading ahead and buffering.

  SelectAlways should be set for files for which events are
  considered to be of higher priority than others. Otherwise, if one
  is interested in just any event, not setting SelectAlways may be a
  little faster. }
function IOSelect (var Events : array [m .. n : Natural] of IOSelectType; MicroSeconds : MicroSecondTimeType) : Integer; asmname '_p_ioselect';

{ A simpler interface to SelectIO for the most common use. Waits for
  SelectReadOrEOF on all files and returns an index. }
function IOSelectRead (const Files : array [m .. n : Natural] of PAnyFile; MicroSeconds : MicroSecondTimeType) : Integer; asmname '_p_ioselectread';

procedure AssignFile   (var T : AnyFile; const Name : String);                     asmname '_p_assign';
procedure AssignBinary (var T : Text; const Name : String);                        asmname '_p_assign_binary';
procedure AssignHandle (var T : AnyFile; Handle : Integer);                        asmname '_p_assign_handle';

{ BP compatible seeking routines }
function  SeekEOF  (var f : Text) : Boolean; asmname '_p_seekeof';
function  SeekEOLn (var f : Text) : Boolean; asmname '_p_seekeoln';

{ Under development }
procedure AnyStringTFDD_Reset (var f : AnyFile; var Buf : ConstAnyString); asmname '_p_anystring_tfdd_reset';
(*procedure AnyStringTFDD_Rewrite (var f : AnyFile; var Buf : VarAnyString); asmname '_p_anystring_tfdd_rewrite';*)
procedure StringTFDD_Reset (var f : AnyFile; var Buf : ConstAnyString; const s : String); asmname '_p_string_tfdd_reset';
(*procedure StringTFDD_Rewrite (var f : AnyFile; var Buf : VarAnyString; var s : String); asmname '_p_string_tfdd_rewrite';*)

{ Generic file handling routines and their support, from file.c }

{ Flags that can be ORed into FileMode. The default value of
  FileMode is FileMode_Reset_ReadWrite. The somewhat confusing
  values are meant to be compatible to BP (as far as BP supports
  them). }
const
  { Allow writing to files opened with Reset }
  FileMode_Reset_ReadWrite   = 2;

  { Do not allow reading from files opened with Rewrite }
  FileMode_Rewrite_WriteOnly = 4;

  { Do not allow reading from files opened with Extend }
  FileMode_Extend_WriteOnly  = 8;

{ File mode constants that are ORed for BindingType.Mode and ChMod.
  The values below are valid for all OSs (as far as supported). If
  the OS uses different values, they're converted internally. }
const
  fmSetUID           = 8#4000;
  fmSetGID           = 8#2000;
  fmSticky           = 8#1000;
  fmUserReadable     = 8#400;
  fmUserWritable     = 8#200;
  fmUserExecutable   = 8#100;
  fmGroupReadable    = 8#40;
  fmGroupWritable    = 8#20;
  fmGroupExecutable  = 8#10;
  fmOthersReadable   = 8#4;
  fmOthersWritable   = 8#2;
  fmOthersExecutable = 8#1;

type
  TextFile = Text;

  StatFSBuffer = record
    BlockSize, BlocksTotal, BlocksFree : LongestInt;
    FilesTotal, FilesFree : Integer
  end;

procedure CloseFile    (          var aFile : (*@@Any*)File);                                        asmname '_p_close';
(*@@IO critical*) procedure ChMod        (          var aFile : AnyFile; Mode : Integer);                        asmname '_p_chmod';
(*@@IO critical*) procedure StatFS       (Path : CString; var Buf : StatFSBuffer);                               asmname '_p_statfs';

{ Checks if data are available to be read from aFile. This is similar to
  `not EOF (aFile)', but does not block on "files" that can grow, like TTYs
  or pipes. }
function  CanRead      (var aFile : AnyFile) : Boolean;                                        asmname '_p_canread';

{ Get the file handle. }
function  FileHandle   (protected var aFile : AnyFile) : Integer;                                        asmname '_p_filehandle';

{ The following routine is meant *only* as a work-around for a
  problem under IRIX where writing reals with Write/Writeln/WriteStr
  does not work. This function will disappear after the problem has
  been solved. Width and Prec are the formatting parameters
  specified after `:'s in Write. If not wanted, NoWidth and
  NoPrecision can be passed. }
const
  NoWidth = Low (Integer);
  NoPrecision = Low (Integer);
function LongReal2Str (Num : LongReal; Width, Prec : Integer) : TString; asmname '_p_longreal2str';

{ File name routines, from filename.pas }

{
  Define constants for different systems:

  OSDosFlag:         flag to indicate whether the target system is
                     Dos

  QuotingCharacter:  the character used to quote wild cards and
                     other special characters (#0 if not available)

  PathSeparator:     the separator of multiple paths, e.g. in the
                     PATH environment variable

  DirSeparator:      the separator of the directories within a full
                     file name

  DirSeparators:     a set of all possible directory and drive name
                     separators

  ExtSeparator:      the separator of a file name extension

  DirRoot:           the name of the root directory

  DirSelf:           the name of a directory in itself

  DirParent:         the name of the parent directory

  NullDeviceName:    the full file name of the null device

  TTYDeviceName:     the full file name of the current TTY

  ConsoleDeviceName: the full file name of the system console. On
                     Dos systems, this is the same as the TTY, but
                     on systems that allow remote login, this is a
                     different thing and may reach a completely
                     different user than the one running the
                     program, so use with care.

  EnvVarCharsFirst:  the characters accepted at the beginning of the
                     name of an environment variable without quoting

  EnvVarChars:       the characters accepted in the name of an
                     environment variable without quoting

  PathEnvVar:        the name of the environment variable which
                     (usually) contains the executable search path

  ShellEnvVar:       the name of the environment variable which
                     (usually) contains the path of the shell
                     executable (see GetShellPath)

  ShellExecCommand:  the option to the (default) shell to execute
                     the command specified in the following argument
                     (see GetShellPath)

  ConfigFileMask:    a mask for the option file name as returned by
                     ConfigFileName

  FileNamesCaseSensitive:
                     flag to indicate whether file names are case
                     sensitive
}

const
  UnixShellEnvVar        = 'SHELL';
  UnixShellExecCommand   = '-c';

{$ifdef __OS_DOS__}

const
  OSDosFlag              = True;
  QuotingCharacter       = #0;
  PathSeparator          = {$ifdef __CYGWIN32__} ':' {$else} ';' {$endif};
  DirSeparator           = '\';
  DirSeparators          = [':', '\', '/'];
  ExtSeparator           = '.';
  DirRoot                = '\';
  DirSelf                = '.';
  DirParent              = '..';
  NullDeviceName         = 'nul';
  TTYDeviceName          = 'con';
  ConsoleDeviceName      = 'con';
  EnvVarCharsFirst       = ['A' .. 'Z', 'a' .. 'z', '_'];
  EnvVarChars            = EnvVarCharsFirst + ['0' .. '9'];
  PathEnvVar             = 'PATH';
  ShellEnvVar            = 'COMSPEC';
  ShellExecCommand       = '/c';
  ConfigFileMask         = '*.cfg';
  FileNamesCaseSensitive = False;

{$else}

const
  OSDosFlag              = False;
  QuotingCharacter       = '\';
  PathSeparator          = ':';
  DirSeparator           = '/';
  DirSeparators          = ['/'];
  ExtSeparator           = '.';
  DirRoot                = '/';
  DirSelf                = '.';
  DirParent              = '..';
  NullDeviceName         = '/dev/null';
  TTYDeviceName          = '/dev/tty';
  ConsoleDeviceName      = '/dev/console';
  EnvVarCharsFirst       = ['A' .. 'Z', 'a' .. 'z', '_'];
  EnvVarChars            = EnvVarCharsFirst + ['0' .. '9'];
  PathEnvVar             = 'PATH';
  ShellEnvVar            = UnixShellEnvVar;
  ShellExecCommand       = UnixShellExecCommand;
  ConfigFileMask         = '.*';
  FileNamesCaseSensitive = True;

{$endif}

const
  WildCardChars = ['*', '?', '[', ']'];
  FileNameSpecialChars = (WildCardChars + SpaceCharacters + ['{', '}', '$', QuotingCharacter]) - DirSeparators;

type
  DirPtr = Pointer;

  PPStrings = ^TPStrings;
  TPStrings (Count : Cardinal) = array [1 .. Count] of ^String;

  GlobBuffer = record
    Count    : Integer;
    Result   : PCStrings;
    Internal : Pointer
  end;

{ Convert ch to lower case if FileNamesCaseSensitive is False, leave
  it unchanged otherwise. }
function  FileNameLoCase (ch : Char) : Char;                               asmname '_p_filenamelocase';

{ Change a file name to use the OS dependent directory separator }
function  Slash2OSDirSeparator (const s : String) : TString;               asmname '_p_slash2osdirseparator';

{ Change a file name to use '/' as directory separator }
function  OSDirSeparator2Slash (const s : String) : TString;               asmname '_p_osdirseparator2slash';

{ Like Slash2OSDirSeparator for CStrings -- NOTE: overwrites the
  CString }
function  Slash2OSDirSeparator_CString (s : CString) : CString;            asmname '_p_slash2osdirseparator_cstring';

{ Like OSDirSeparator2Slash for CStrings -- NOTE: overwrites the
  CString }
function  OSDirSeparator2Slash_CString (s : CString) : CString;            asmname '_p_osdirseparator2slash_cstring';

{ Add a DirSeparator to the end of s, if there is not already one
  and s denotes an existing directory }
function  AddDirSeparator (const s : String) : TString;                    asmname '_p_adddirseparator';

{ Like AddDirSeparator, but also if the directory does not exist }
function  ForceAddDirSeparator (const s : String) : TString;               asmname '_p_forceadddirseparator';

{ Remove all trailing DirSeparators from s, if there are any, as
  long as removing them doesn't change the meaning (i.e., they don't
  denote the root directory. }
function  RemoveDirSeparator (const s : String) : TString;                 asmname '_p_removedirseparator';

{ Returns the current directory using OS dependent directory
  separators }
function  GetCurrentDirectory     : TString;                               asmname '_p_get_current_directory';

{ Returns a directory suitable for storing temporary files using OS
  dependent directory separators. If found, the result always ends
  in DirSeparator. If no suitable directory is found, an empty
  string is returned. }
function  GetTempDirectory        : TString;                               asmname '_p_get_temp_directory';

{ Returns a non-existing file name in GetTempDirectory. If no temp
  directory is found, i.e. GetTempDirectory returns the empty
  string, a runtime error is raised. }
(*@@iocritical*)function  GetTempFileName         : TString;                               asmname '_p_get_temp_file_name';

{ The same as GetTempFileName, but returns a CString allocated from
  the heap. }
(*@@iocritical*)function  GetTempFileName_CString : CString;                               asmname '_p_get_temp_file_name_cstring';

{ Get the external name of a file }
function  FileName (var f : AnyFile) : TString;                            asmname '_p_file_name';

{ Returns true if the given file name is an existing plain file }
function  FileExists      (const aFileName : String) : Boolean;            asmname '_p_file_exists';

{ Returns true if the given file name is an existing directory }
function  DirectoryExists (const aFileName : String) : Boolean;            asmname '_p_directory_exists';

{ Returns true if the given file name is an existing file, directory
  or special file (device, pipe, socket, etc.) }
function  PathExists      (const aFileName : String) : Boolean;            asmname '_p_path_exists';

{ If a file of the given name exists in one of the directories given
  in DirList (separated by PathSeparator), returns the full path,
  otherwise returns an empty string. If aFileName already contains
  an element of DirSeparators, returns Slash2OSDirSeparator
  (aFileName) if it exists. }
function  FSearch (const aFileName, DirList : String) : TString;           asmname '_p_fsearch';

{ Like FSearch, but only find executable files. Under Dos, if not
  found, the function tries appending '.com', '.exe' and '.bat', so
  you don't have to specify these extensions in aFileName (and with
  respect to portability, it might be preferable not to do so). }
function  FSearchExecutable (const aFileName, DirList : String) : TString; asmname '_p_fsearch_executable';

{ Replaces all occurrences of `$FOO' and `~' in s by the value of
  the environment variables FOO or HOME, respectively. If a variable
  is not defined, the function returns False, and s contains the
  name of the undefined variable (or the empty string if the
  variable name is invalid, i.e., doesn't start with a character
  from EnvVarCharsFirst). Otherwise, if all variables are found, s
  contains the replaced string, and True is returned. }
function  ExpandEnvironment (var s : String) : Boolean;                    asmname '_p_expand_environment';

{ Expands the given path name to a full path name. Relative paths
  are expanded using the current directory, and occurrences of
  DirSelf and DirParent are resolved. Under Dos, the result is
  converted to lower case and a trailing ExtSeparator (except in a
  trailing DirSelf or DirParent) is removed, like Dos does. If the
  directory, i.e. the path without the file name, is invalid, the
  empty string is returned. }
function  FExpand       (const Path : String) : TString;                   asmname '_p_fexpand';

{ Like FExpand, but unquotes the directory before expanding it, and
  quotes WildCardChars again afterwards. Does not check if the
  directory is valid (because it may contain wild card characters).
  Symlinks are expanded only in the directory part, not the file
  name. }
function  FExpandQuoted (const Path : String) : TString;                   asmname '_p_fexpandquoted';

{ FExpands Path, and then removes the current directory from it, if
  it is a prefix of it. If OnlyCurDir is set, the current directory
  will be removed only if Path denotes a file in, not below, it. }
function  RelativePath (const Path : String; OnlyCurDir, Quoted : Boolean) : TString; asmname '_p_relative_path';

{ Is aFileName a UNC filename? (Always returns False on non-Dos
  systems.) }
function  IsUNC (const aFileName : String) : Boolean;                      asmname '_p_IsUNC';

{ Splits a file name into directory, name and extension }
procedure FSplit (const Path : String; var Dir, Name, Ext : String);       asmname '_p_fsplit';

{ Functions that extract one or two of the parts from FSplit.
  DirFromPath returns DirSelf + DirSeparator if the path contains no
  directory. }
function  DirFromPath     (const Path : String) : TString;                 asmname '_p_dir_from_path';
function  NameFromPath    (const Path : String) : TString;                 asmname '_p_name_from_path';
function  ExtFromPath     (const Path : String) : TString;                 asmname '_p_ext_from_path';
function  NameExtFromPath (const Path : String) : TString;                 asmname '_p_name_ext_from_path';

{ Start reading a directory. If successful, a pointer is returned
  that can be used for subsequent calls to ReadDir and finally
  CloseDir. On failure, an I/O error is raised and (in case it is
  ignored) nil is returned. }
(*@@iocritical*)function  OpenDir  (const Name : String) : DirPtr;                         asmname '_p_opendir';

{ Reads one entry from the directory Dir, and returns the file name.
  On errors or end of directory, the empty string is returned. }
function  ReadDir  (Dir : DirPtr) : TString;                               asmname '_p_readdir';

{ Closes a directory opened with OpenDir. }
(*@@iocritical*)procedure CloseDir (Dir : DirPtr);                                         asmname '_p_closedir';

{ Returns the first position of a non-quoted character of CharSet in
  s, or 0 if no such character exists. }
function  FindNonQuotedChar (Chars : CharSet; const s : String; From : Integer) : Integer; asmname '_p_findnonquotedchar';

{ Returns the first occurence of SubString in s that is not quoted
  at the beginning, or 0 if no such occurence exists. }
function  FindNonQuotedStr (const SubString, s : String; From : Integer) : Integer; asmname '_p_findnonquotedstr';

{ Does a string contain non-quoted wildcard characters? }
function  HasWildCards (const s : String) : Boolean;                       asmname '_p_haswildcards';

{ Does a string contain non-quoted wildcard characters, braces or
  spaces? }
function  HasWildCardsOrBraces (const s : String) : Boolean;               asmname '_p_haswildcardsorbraces';

{ Insert quoting QuotingCharacter into s before any special characters }
function  QuoteFileName (const s : String; const SpecialCharacters : CharSet) : TString; asmname '_p_quote_filename';

{ Remove quoting QuotingCharacter from s }
function  UnQuoteFileName (const s : String) : TString;                    asmname '_p_unquote_filename';

{ Splits s at non-quoted spaces and expands non-quoted braces like
  bash does. The result and its entries should be Disposed after
  usage. }
function  BraceExpand (const s : String) : PPStrings;                      asmname '_p_braceexpand';

{ Tests if a file name matches a shell wildcard pattern (?, *, []) }
function  FileNameMatch (const Pattern, Name : String) : Boolean;          asmname '_p_filenamematch';

{ FileNameMatch with BraceExpand }
function  MultiFileNameMatch (const Pattern, Name : String) : Boolean;     asmname '_p_multifilenamematch';

{ File name globbing }
{ GlobInit is implied by Glob and MultiGlob, not by GlobOn and
  MultiGlobOn. GlobOn and MultiGlobOn must be called after GlobInit,
  Glob or MultiGlob. MultiGlob and MultiGlobOn do brace expansion,
  Glob and GlobOn do not. GlobFree frees the memory allocated by the
  globbing functions and invalidates the results in Buf. It should
  be called after globbing. }
procedure GlobInit    (var Buf : GlobBuffer);                              asmname '_p_globinit';
procedure Glob        (var Buf : GlobBuffer; Pattern : CString);           asmname '_p_glob';
procedure GlobOn      (var Buf : GlobBuffer; Pattern : CString);           asmname '_p_globon';
procedure MultiGlob   (var Buf : GlobBuffer; Pattern : String);            asmname '_p_multiglob';
procedure MultiGlobOn (var Buf : GlobBuffer; Pattern : String);            asmname '_p_multiglobon';
procedure GlobFree    (var Buf : GlobBuffer);                              asmname '_p_globfree';

type
  TPasswordEntry = record
    UserName, RealName, Password, HomeDirectory, Shell : TString;
    UID, GID : Integer
  end;

  PPasswordEntries = ^TPasswordEntries;
  TPasswordEntries (Count : Integer) = array [1 .. Count] of TPasswordEntry;

{ Finds a password entry by user name. Returns True if found, False
  otherwise. }
function  GetPasswordEntryByName (const UserName : String; var Entry : TPasswordEntry) : Boolean; asmname '_p_getpasswordentrybyname';

{ Finds a password entry by UID. Returns True if found, False
  otherwise. }
function  GetPasswordEntryByUID  (UID : Integer; var Entry : TPasswordEntry) : Boolean; asmname '_p_getpasswordentrybyuid';

{ Returns all password entries, or nil if none found. }
function  GetPasswordEntries : PPasswordEntries; asmname '_p_getpasswordentries';

{ Returns the mount point (Unix) or drive (Dos) which is part of the
  given path. If the path does not contain any (i.e., is a relative
  path), an empty string is returned. Therefore, if you want to get
  the mount point or drive in any case, apply `FExpand' or
  `RealPath' to the argument. }
function  GetMountPoint (const Path : String) = Result : TString; asmname '_p_GetMountPoint';

{ Returns the path to the shell (as the return value) and the option
  that makes it execute the command specified in the following
  argument (in `Option'). Usually these are the environment value of
  ShellEnvVar, and ShellExecCommand, but on Dos systems, the
  function will first try UnixShellEnvVar, and UnixShellExecCommand
  because ShellEnvVar will usually point to command.com, but
  UnixShellEnvVar can point to bash which is usually a better choice
  when present. If UnixShellEnvVar is not set, or the shell given
  does not exist, it will use ShellEnvVar, and ShellExecCommand.
  Option may be null (in case you want to invoke the shell
  interactively). }
function  GetShellPath (var Option : String) : TString; asmname '_p_GetShellPath';

{ Returns the path of the running executable. NOTE: On most systems,
  this is *not* guaranteed to be the full path, but often just the
  same as `ParamStr (0)' which usually is the name given on the
  command line. Only on some systems with special support, it
  returns the full path when `ParamStr (0)' doesn't. }
function  ExecutablePath : TString; asmname '_p_executable_path';

{ Returns a file name suitable for a global (system-wide) or local
  (user-specific) configuration file, depending on the Global
  parameter. The function does not guarantee that the file name
  returned exists or is readable or writable.

  In the following table, the base name `<base>' is given with the
  Name parameter. If it is empty, the base name is the name of the
  running program (as returned by ExecutablePath, without directory
  and extension. `<prefix>' (Unix only) stands for the value of the
  Prefix parameter (usual values include '', '/usr' and
  '/usr/local'). `<dir>' (Dos only) stands for the directory where
  the running program resides. `$foo' stands for the value of the
  environment variable `foo'.

         Global                    Local
  Unix:  <prefix>/etc/<base>.conf  $HOME/.<base>

  Dos:   $DJDIR\etc\<base>.ini     $HOME\<base>.cfg
         <dir>\<base>.ini          <dir>\<base>.cfg

  As you see, there are two possibilities under Dos. If the first
  file exists, it is returned. Otherwise, if the second file exists,
  that is returned. If none of them exists (but the program might
  want to create a file), if the environment variable (DJDIR or
  HOME, respectively) is set, the first file name is returned,
  otherwise the second one. This rather complicated scheme should
  give the most reasonable results for systems with or without DJGPP
  installed, and with or without already existing config files. Note
  that DJDIR is always set on systems with DJGPP installed, while
  HOME is not. However, it is easy for users to set it if they want
  their config files in a certain directory rather than with the
  executables. }
function  ConfigFileName (const Prefix, Name : String; Global : Boolean) : TString; asmname '_p_config_file_name';

{ Returns a directory name suitable for global, machine-independent
  data. The function garantees that the name returned ends with a
  DirSeparator, but does not guarantee that it exists or is
  readable or writable.

  Note: If the prefix is empty, it is assumed to be '/usr'. (If you
  really want /share, you could pass '/' as the prefix, but that's
  very uncommon.)

  Unix: <prefix>/share/<base>/

  Dos:  $DJDIR\share\<base>\
        <dir>\

  About the symbols used above, and the two possibilities under Dos,
  see the comments for ConfigFileName. }
function  DataDirectoryName (const Prefix, Name : String) : TString; asmname '_p_data_directory_name';

{ ==================== MATHEMATICAL ROUTINES ===================== }

function  IsInfinity   (x : Extended) : Boolean; attribute (const); asmname '_p_isinf';
function  IsNotANumber (x : Extended) : Boolean; attribute (const); asmname '_p_isnan';
procedure SplitReal    (x : Extended; var Exponent : Integer; var Mantissa : Extended); asmname '_p_frexp';
function  SinH         (x : Double)    : Double; asmname '_p_sinh';
function  CosH         (x : Double)    : Double; asmname '_p_cosh';
function  Arctan2      (y, x : Double) : Double; asmname '_p_arctan2';

type
  RandomSeedType = Cardinal (32);
  RandomizeType  = ^procedure;
  SeedRandomType = ^procedure (Seed : RandomSeedType);
  RandRealType   = ^function : LongestReal;
  RandIntType    = ^function (MaxValue : LongestCard) : LongestCard;

var
  RandomizePtr  : RandomizeType; asmname '_p_randomize_ptr'; external;
  SeedRandomPtr : SeedRandomType; asmname '_p_seedrandom_ptr'; external;
  RandRealPtr   : RandRealType; asmname '_p_randreal_ptr'; external;
  RandIntPtr    : RandIntType; asmname '_p_randint_ptr'; external;

procedure SeedRandom (Seed : RandomSeedType); asmname '_p_seedrandom';

end.

module GPC implementation;

{$ifndef HAVE_NO_RTS_CONFIG_H}
{$include "rts-config.inc"}
{$endif}
{$ifdef HAVE_LIBOS_HACKS}
{$L os-hacks}
{$endif}

end.
