/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * loadamf2.c - Code to read a DSMI AMF module file,  / / \  \
 *              opening and closing it for you, and  | <  /   \_
 *              do an initial run-through.           |  \/ /\   /
 *                                                    \_  /  > /
 *                                                      | \ / /
 * By Christopher Snowhill.                             |  ' /
 *                                                       \__/
 */

#include "dumb.h"
#include "internal/it.h"

/* dumb_load_amf(): loads a AMF file into a DUH struct, returning a pointer
 * to the DUH struct. When you have finished with it, you must pass the
 * pointer to unload_duh() so that the memory can be freed.
 */
DUH *dumb_load_amf(const char *filename) {
    DUH *duh = dumb_load_amf_quick(filename);
    dumb_it_do_initial_runthrough(duh);
    return duh;
}
