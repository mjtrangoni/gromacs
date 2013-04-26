/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2013, by the GROMACS development team, led by
 * David van der Spoel, Berk Hess, Erik Lindahl, and including many
 * others, as listed in the AUTHORS file in the top-level source
 * directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \internal \file
 * \brief
 * Implements test helper routines from toputils.h.
 *
 * \author Teemu Murtola <teemu.murtola@gmail.com>
 * \ingroup module_selection
 */
#include "toputils.h"

#include <cstring>

#include "gromacs/legacyheaders/smalloc.h"
#include "gromacs/legacyheaders/statutil.h"
#include "gromacs/legacyheaders/tpxio.h"
#include "gromacs/legacyheaders/vec.h"

#include "gromacs/utility/gmxassert.h"

#include "testutils/testfilemanager.h"

namespace gmx
{
namespace test
{

TopologyManager::TopologyManager()
    : top_(NULL), frame_(NULL)
{
}

TopologyManager::~TopologyManager()
{
    if (top_ != NULL)
    {
        free_t_atoms(&top_->atoms, TRUE);
        done_top(top_);
        sfree(top_);
    }

    if (frame_ != NULL)
    {
        sfree(frame_->x);
        sfree(frame_);
    }
}

void TopologyManager::requestFrame()
{
    GMX_RELEASE_ASSERT(top_ == NULL,
                       "Frame must be requested before initializing topology");
    if (frame_ == NULL)
    {
        snew(frame_, 1);
    }
}

void TopologyManager::loadTopology(const char *filename)
{
    char    title[STRLEN];
    int     ePBC;
    rvec   *xtop = NULL;
    matrix  box;

    GMX_RELEASE_ASSERT(top_ == NULL, "Topology initialized more than once");
    snew(top_, 1);
    read_tps_conf(gmx::test::TestFileManager::getInputFilePath(filename).c_str(),
                  title, top_, &ePBC, frame_ != NULL ? &xtop : NULL,
                  NULL, box, FALSE);

    if (frame_ != NULL)
    {
        frame_->flags  = TRX_NEED_X;
        frame_->natoms = top_->atoms.nr;
        frame_->bX     = TRUE;
        snew(frame_->x, frame_->natoms);
        std::memcpy(frame_->x, xtop, sizeof(*frame_->x) * frame_->natoms);
        frame_->bBox   = TRUE;
        copy_mat(box, frame_->box);
    }

    sfree(xtop);
}

} // namespace test
} // namespace gmx