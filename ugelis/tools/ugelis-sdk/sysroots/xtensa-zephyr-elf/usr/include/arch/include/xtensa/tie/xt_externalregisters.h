/* Definitions for the xt_externalregisters TIE package */

/*
 * Customer ID=4313; Build=0x64d47; Copyright (c) 2004-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

/* Do not modify. This is automatically generated.*/

#ifndef _XTENSA_xt_externalregisters_HEADER
#define _XTENSA_xt_externalregisters_HEADER

#ifdef __XTENSA__
#ifdef __XCC__

#ifndef _ASMLANGAGE
#ifndef _NOCLANGUAGE
#ifndef __ASSEMBLER__

#include <xtensa/tie/xt_core.h>

/*
 * The following prototypes describe intrinsic functions
 * corresponding to TIE instructions.  Some TIE instructions
 * may produce multiple results (designated as "out" operands
 * in the iclass section) or may have operands used as both
 * inputs and outputs (designated as "inout").  However, the C
 * and C++ languages do not provide syntax that can express
 * the in/out/inout constraints of TIE intrinsics.
 * Nevertheless, the compiler understands these constraints
 * and will check that the intrinsic functions are used
 * correctly.  To improve the readability of these prototypes,
 * the "out" and "inout" parameters are marked accordingly
 * with comments.
 */

extern unsigned _TIE_xt_externalregisters_RSR_ERACCESS(void);
extern void _TIE_xt_externalregisters_WSR_ERACCESS(unsigned art);
extern void _TIE_xt_externalregisters_XSR_ERACCESS(unsigned art /*inout*/);
extern unsigned _TIE_xt_externalregisters_RER(unsigned ars);
extern void _TIE_xt_externalregisters_WER(unsigned art, unsigned ars);

#endif /*_ASMLANGUAGE*/
#endif /*_NOCLANGUAGE*/
#endif /*__ASSEMBLER__*/

#define XT_RSR_ERACCESS _TIE_xt_externalregisters_RSR_ERACCESS
#define XT_WSR_ERACCESS _TIE_xt_externalregisters_WSR_ERACCESS
#define XT_XSR_ERACCESS _TIE_xt_externalregisters_XSR_ERACCESS
#define XT_RER _TIE_xt_externalregisters_RER
#define XT_WER _TIE_xt_externalregisters_WER

#endif /* __XCC__ */

#endif /* __XTENSA__ */

#endif /* !_XTENSA_xt_externalregisters_HEADER */
