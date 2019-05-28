//
//  PlannerTestBase.cpp
//  TestLib
//
//  Created by Eric Zinda on 3/24/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include <stdio.h>
#include "FXPlatform/Prolog/HtnGoalResolver.h"

string DiffSolutionInOrder(shared_ptr<HtnTermFactory> factory, const string &expected, shared_ptr<vector<UnifierType>> solution);
