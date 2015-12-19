/****************************************************************************
 * arch/arm/src/tms570/tms570_clockconfig.c
 *
 *   Copyright (C) 2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * This is primarily original code.  However, some logic in this file was
 * inspired/leveraged from TI's Project0 which has a compatible BSD license
 * and credit should be given in any case:
 *
 *   Copyright (c) 2012, Texas Instruments Incorporated
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <arch/board/board.h>

#include "up_arch.h"

#include "chip/tms570_sys.h"
#include "tms570_clockconfig.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void tms570_pll_setup(void)
{
  uint32_t regval;

  /* Configure PLL control registers */

  /* Setup pll control register 1
   *
   * REFCLKDIV controls input clock divider:
   *
   *  NR = REFCLKDIV+1
   *  Fintclk = Fclkin / NR
   *
   * PLLMUL controls multipler on divided input clock (Fintclk):
   *
   *  Non-modulated:
   *    NF = (PLLMUL + 256) / 256
   *  Modulated:
   *    NF = (PLLMUL + MULMOD + 256) / 256
   *
   *  Foutputclk = Fintclk x NF (150MHz - 550MHz)
   *
   * ODPLL controls internal PLL output divider:
   *
   *   OD = ODPLL+1
   *   Fpostodclk = Foutputclock / OD
   *
   * Final divisor, R, controls PLL output:
   *
   *   R = PLLDIV + 1
   *   Fpllclock = Fpostodclk / R
   *
   * Or:
   *
   *   Fpllclock = = (Fclkin / NR) x NF / OD / R
   *
   * For example, if the clock source is a 16MHz crystal, then
   *
   *   Fclkin = 16,000,000
   *   NR     = 6   (REFCLKDIV=5)
   *   NF     = 120 (PLLMUL = 119 * 256)
   *   OD     = 1   (ODPLL = 0)
   *   R      = 32  (PLLDIV=31)
   *
   * Then:
   *
   *   Fintclk      = 16 MHz / 6 = 2.667 MHz
   *   Foutputclock = 2.667 MHz * 120 = 320 MHz
   *   Fpostodclock = 320 MHz / 2
   *   Fpllclock    = 160 MHz / 2 = 80 MHz
   *
   * NOTE: That R is temporary set to the maximum (32) here.
   */

  regval = SYS_PLLCTL1_PLLMUL((BOARD_PLL_NF - 1) << 8) |
           SYS_PLLCTL1_REFCLKDIV(BOARD_PLL_NR - 1) |
           SYS_PLLCTL1_PLLDIV_MAX |
           SYS_PLLCTL1_MASKSLIP_DISABLE;
  putreg32(regval, TMS570_SYS_PLLCTL1);

  /* Setup pll control register 2 */

  regval = SYS_PLLCTL2_SPRAMOUNT(61) |
           SYS_PLLCTL2_ODPLL(BOARD_PLL_OD - 1) |
           SYS_PLLCTL2_MULMOD(7) |
           SYS_PLLCTL2_SPRRATE(255);
  putreg32(regval, TMS570_SYS_PLLCTL2);

  /* Enable PLL(s) to start up or Lock.
   *
   * On wakeup, only clock sources 0, 4, and 5 are enabled: Oscillator, Low
   * and hight Frequency LPO.  Clear bit 1 to enable the PLL.  Only the
   * external clock remains disabled.
   */

  regval = SYS_CLKSRC_EXTCLKIN;
  putreg32(regval, TMS570_SYS_CSDIS);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: tms570_clockconfig
 *
 * Description:
 *   Called to initialize TMS570 clocking.  This does whatever setup is
 *   needed to put the SoC in a usable state.  This includes, but is not
 *   limited to, the initialization of clocking using the settings in the
 *   board.h header file.
 *
 ****************************************************************************/

void tms570_clockconfig(void)
{
  /* Configure PLL control registers and enable PLLs. The PLL takes (127 +
   * 1024  NR) oscillator cycles to acquire lock. This initialization
   * sequence performs all the tasks that are not required to be done at
   * full application speed while the PLL locks.
   */

   tms570_pll_setup();

#ifdef CONFIG_TMS570_SELFTEST
  /* Run eFuse controller start-up checks and start eFuse controller ECC
   * self-test.  This includes a check for the eFuse controller error
   * outputs to  be stuck-at-zero.
   */

#  warning Missing Logic
#endif /* CONFIG_TMS570_SELFTEST */

  /* Enable clocks to peripherals and release peripheral reset */
#  warning Missing Logic

  /* Configure device-level multiplexing and I/O multiplexing */
#  warning Missing Logic

#ifdef CONFIG_TMS570_SELFTEST
  /* Wait for eFuse controller self-test to complete and check results */
#  warning Missing Logic

#endif

  /* Set up flash address and data wait states based on the target CPU clock
   * frequency The number of address and data wait states for the target CPU
   * clock frequency are specified in the specific part's datasheet.
   */
#warning Missing Logic

  /* Configure the LPO such that HF LPO is as close to 10MHz as possible */
#warning Missing Logic

  /* Wait for PLLs to start up and map clock domains to desired clock
   * sources.
   */
#warning Missing Logic

  /* Set ECLK pins functional mode */
#warning Missing Logic

  /* Set ECLK pins default output value */
#warning Missing Logic

  /* Set ECLK pins output direction */
#warning Missing Logic

  /* Set ECLK pins open drain enable */
#warning Missing Logic

  /* Set ECLK pins pullup/pulldown enable */
#warning Missing Logic

  /* Set ECLK pins pullup/pulldown select */
#warning Missing Logic

  /* Setup ECLK */
#warning Missing Logic
}
