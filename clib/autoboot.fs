\ Copyright 2021 Bradley D. Nelson
\
\ Licensed under the Apache License, Version 2.0 (the "License");
\ you may not use this file except in compliance with the License.
\ You may obtain a copy of the License at
\
\     http://www.apache.org/licenses/LICENSE-2.0
\
\ Unless required by applicable law or agreed to in writing, software
\ distributed under the License is distributed on an "AS IS" BASIS,
\ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\ See the License for the specific language governing permissions and
\ limitations under the License.

( Setup entry )
internals : ok   ." uEforth[clib]" raw-ok ; forth \ prompt refill drop quit ; forth

(
internals definitions also ansi

: boot-set-title   s" uEforth" set-title ;
' boot-set-title boot-prompt !

only forth definitions
)

' clibtype is type
' clibkey is key
' clibbye is bye
' clibhaskey is key?

( Include first argument if any )
internals definitions

( Leave some room for growth of starting system. )
0 value saving-base
: park-heap ( -- a ) saving-base ;
: park-forth ( -- a ) saving-base cell+ ;
: 'cold ( -- a ) saving-base 2 cells + ;
: setup-saving-base
  here to saving-base  16 cells allot  0 'cold ! ;

' forth >body constant forth-wordlist


: autoexec
   ( Open passed file if any. )
   \ argc 2 >= if 1 argv included exit then
   ( Open remembered file if any. )
   \ ['] revive catch drop
;

' autoexec ( leave on dstack for fini.fs )
forth definitions
