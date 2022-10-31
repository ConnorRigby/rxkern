! based on KPIT GNUSH
! This moves the whole payload up to the desired address,

! (c) copyright fenugrec 2016
! GPLv3
! 
! This program is free software: you can redistribute it and/or modify
! it under the terms of the GNU General Public License as published by
! the Free Software Foundation, either version 3 of the License, or
! (at your option) any later version.
! 
! This program is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! GNU General Public License for more details.
! 
! You should have received a copy of the GNU General Public License
! along with this program.  If not, see <http://www.gnu.org/licenses/>.
!



	.list
	.section .rja
	.global RAMjump_entry

	.extern _rja_start
	.extern _main	!user code
	.extern _ebss
	.extern _bss
	.extern _stackinit
	.extern _endpayload

RAMjump_entry:
	mov.w @r13 => 0xffffe406,r2
	extu.w r2,r2
	tst r14,r2
	bf RAMjump_entry
	mov.w @(ffffa3a2,pc),r1
	mov.w r14,@r1=>0xffffe40a
	mov.w @(ffffa3a4,pc),r7=ffffe528
	mov #0x0,r5
	mov.l @(->Can_TX,pc),r3
	mov.l @r15=>,r6
	jsr @r3=>CAN_TX
	mov #0x8,payload
	mov.w r14,@r13=>0xffffe406
	add 0x4,r15
	lds.l @r15=>local_c+,pr

	.BALIGN 4
stack:
		.long	_stackinit
main:
		.long	_main
bss:
		.long	_sbss
ebss:
		.long	_ebss
