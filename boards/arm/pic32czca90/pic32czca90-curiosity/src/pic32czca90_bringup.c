/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/src/pic32czca90_bringup.c
 *
 * Board-specific initialization
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/mount.h>
#include <stdio.h>
#include <syslog.h>

#include <nuttx/board.h>
#include <nuttx/fs/fs.h>

#ifdef CONFIG_FS_PROCFS
#  include <nuttx/fs/procfs.h>
#endif

#include "pic32czca90-curiosity.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: pic32czca90_bringup
 *
 * Description:
 *   Perform architecture-specific initialization.
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y:
 *     Called from board_late_initialize()
 *
 *   CONFIG_NSH_ARCHINIT=y:
 *     Called from the NSH library
 *
 ****************************************************************************/

int pic32czca90_bringup(void)
{
  int ret = OK;

#ifdef CONFIG_FS_PROCFS
  /* Mount the proc filesystem */

  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to mount procfs: %d\n", ret);
    }
#endif

  return ret;
}
