/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2013 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed-code.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "ActionPilot.h"

using namespace std;
namespace PLMD{

void ActionPilot::registerKeywords(Keywords& keys){
}

ActionPilot::ActionPilot(const ActionOptions&ao):
Action(ao),
stride(1)
{
  parse("STRIDE",stride);
  log.printf("  with stride %d\n",stride);
}

bool ActionPilot::onStep()const{
  return getStep()%stride==0;
}

int ActionPilot::getStride()const{
  return stride;
}
}


