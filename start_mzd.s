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
	mov.l	main,r1
	mov.l	stack,r15
	jsr     @r1
	nop

	.BALIGN 4
stack:
		.long	_stackinit
main:
		.long	_main
bss:
		.long	_sbss
ebss:
		.long	_ebss
