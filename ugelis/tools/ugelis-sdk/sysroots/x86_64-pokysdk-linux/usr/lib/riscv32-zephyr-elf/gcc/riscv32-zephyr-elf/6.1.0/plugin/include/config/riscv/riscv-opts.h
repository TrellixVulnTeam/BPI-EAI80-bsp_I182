/* Definition of RISC-V target for GNU compiler.
   Copyright (C) 2016 Free Software Foundation, Inc.
   Contributed by Andrew Waterman (andrew@sifive.com).

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_RISCV_OPTS_H
#define GCC_RISCV_OPTS_H

enum riscv_float_abi_type {
  FLOAT_ABI_SOFT,
  FLOAT_ABI_SINGLE,
  FLOAT_ABI_DOUBLE
};
extern enum riscv_float_abi_type riscv_float_abi;

enum riscv_code_model {
  CM_MEDLOW,
  CM_MEDANY,
  CM_PIC
};
extern enum riscv_code_model riscv_cmodel;

#endif /* ! GCC_RISCV_OPTS_H */
