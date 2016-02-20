// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include <OP/OP_OperatorTable.h>

#include "SOP_FabricGenerator.h"
#include "SOP_FabricDeformer.h"
#include "OBJ_FabricKinematic.h"

void newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OpenSpliceHoudini::OP_SOP_FabricGenerator);
    table->addOperator(new OpenSpliceHoudini::OP_SOP_FabricDeformer);
}

void newObjectOperator(OP_OperatorTable* table)
{
    table->addOperator(new OpenSpliceHoudini::OP_OBJ_FabricKinematic);
}
