// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __OBJ_FABRICSDFG_H__
#define __OBJ_FABRICSDFG_H__

#include <OBJ/OBJ_Geometry.h>
#include "core/FabricDFGOP.h"

namespace OpenSpliceHoudini
{

/// An simple example of Fabric Canvas integration for Houdini Objects
/// It will set the object transform if the Canvas graph got output ports
/// of type Vec3 called "t", "r" or "s" to set translation, rotation and scale  
class OBJ_FabricKinematic : public FabricDFGOP<OBJ_Geometry>
{
public:
    static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*);
    static OP_TemplatePair* buildTemplatePair(OP_TemplatePair* prevstuff);

protected:
    OBJ_FabricKinematic(OP_Network* net, const char* name, OP_Operator* op);
    virtual ~OBJ_FabricKinematic();
    // // Performs the calculation of the local and the world transformation.
    // virtual OP_ERROR cookMyObj(OP_Context& context);

    virtual int applyInputIndependentTransform(OP_Context& context, UT_DMatrix4& mat);
};

class OP_OBJ_FabricKinematic : public OP_Operator
{
public:
    OP_OBJ_FabricKinematic()
        : OP_Operator("fabricObject",                      // Internal name
                      "Fabric Object",                     // UI name
                      OBJ_FabricKinematic::myConstructor,        // How to build the SOP
                      OBJ_FabricKinematic::buildTemplatePair(0), // My parameters
                      0,                                   // Min # of node inputs
                      1,                                   // Max # of node inputs
                      0)                                   // Local variables
    {
    }
};

} // End namespace OpenSpliceHoudini
#endif // __OBJ_FABRICSDFG_H__
