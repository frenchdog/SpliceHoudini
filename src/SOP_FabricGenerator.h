// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __SOP_FABRICGENERATOR_H__
#define __SOP_FABRICGENERATOR_H__

#include <SOP/SOP_Node.h>
#include "core/FabricDFGOP.h"

namespace OpenSpliceHoudini
{

/// Create geometry from a Canvas graph. Only PolygonMesh type supported for the moment.
/// For each output port of type PolygonMesh or PolygonMesh[], the geometry will be copied to the Houdini geo detail
class SOP_FabricGenerator : public FabricDFGOP<SOP_Node>
{
public:
    static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*);

protected:
    SOP_FabricGenerator(OP_Network* net, const char* name, OP_Operator* op);
    virtual ~SOP_FabricGenerator();
    virtual OP_ERROR cookMySop(OP_Context& context);

private:
    void setPolygonMesh(FabricCore::RTVal rtMesh);
    size_t m_groupIndex;
};

class OP_SOP_FabricGenerator : public OP_Operator
{
public:
    OP_SOP_FabricGenerator()
        : OP_Operator("fabricGenerator",                   // Internal name
                      "Fabric Generator",                  // UI name
                      SOP_FabricGenerator::myConstructor,  // How to build the SOP
                      SOP_FabricGenerator::myTemplateList, // My parameters
                      0,                                   // Min # of node inputs
                      0,                                   // Max # of node inputs
                      0,                                   // Local variables
                      OP_FLAG_GENERATOR)                   // Flag it as generator
    {
    }
};

} // End namespace OpenSpliceHoudini
#endif // __SOP_FABRICGENERATOR_H__
