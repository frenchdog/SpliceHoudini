// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "SOP_FabricGenerator.h"

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <CH/CH_LocalVariable.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <SYS/SYS_Math.h>
#include <limits.h>
#include <GU/GU_PrimPoly.h>
#include <GEO/GEO_PolyCounts.h>

#include <ImathVec.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;

OP_Node* SOP_FabricGenerator::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_FabricGenerator(net, name, op);
}

SOP_FabricGenerator::SOP_FabricGenerator(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<SOP_Node>(net, name, op)
    , m_groupIndex(0)
{
}

SOP_FabricGenerator::~SOP_FabricGenerator()
{
}

void SOP_FabricGenerator::setPolygonMesh(FabricCore::RTVal rtVal)
{
    if (rtVal.isArray())
    {
        for (size_t i = 0; i < rtVal.getArraySize(); ++i)
        {
            setPolygonMesh(rtVal.getArrayElementRef(i));
        }
    }
    else
    {
        size_t nbPoints = 0;
        size_t nbPolygons = 0;
        size_t nbSamples = 0;
        if (rtVal.isValid() && !rtVal.isNullObject())
        {
            FabricCore::Client client = getView().getClient();

            nbPoints = rtVal.callMethod("UInt64", "pointCount", 0, 0).getUInt64();
            nbPolygons = rtVal.callMethod("UInt64", "polygonCount", 0, 0).getUInt64();
            nbSamples = rtVal.callMethod("UInt64", "polygonPointsCount", 0, 0).getUInt64();

            if (nbPoints == 0 || nbPolygons == 0 || nbSamples == 0)
            {
                return;
            }

            std::vector<UT_Vector3F> posBuffer(nbPoints);

            {
                std::vector<FabricCore::RTVal> args(2);
                args[0] =
                    FabricCore::RTVal::ConstructExternalArray(client, "Float32", posBuffer.size() * 3, &posBuffer[0]);
                args[1] = FabricCore::RTVal::ConstructUInt32(client, 3); // components

                try
                {
                    rtVal.callMethod("", "getPointsAsExternalArray", 2, &args[0]);
                }
                catch (FabricCore::Exception e)
                {
                    FabricCore::Exception::Throw(e.getDesc_cstr());
                }
            }

            std::vector<int> faceCounts(nbPolygons);
            std::vector<int> elementsIndices(nbSamples);
            {
                std::vector<FabricCore::RTVal> args(2);
                args[0] =
                    FabricCore::RTVal::ConstructExternalArray(client, "UInt32", faceCounts.size(), &faceCounts[0]);
                args[1] = FabricCore::RTVal::ConstructExternalArray(
                    client, "UInt32", elementsIndices.size(), &elementsIndices[0]);

                try
                {
                    rtVal.callMethod("", "getTopologyAsCountsIndicesExternalArrays", 2, &args[0]);
                }
                catch (FabricCore::Exception e)
                {
                    FabricCore::Exception::Throw(e.getDesc_cstr());
                }
            }

            GA_Offset ptoff = gdp->appendPointBlock(nbPoints);
            GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, "P"));
            handle.setBlock(ptoff, nbPoints, &posBuffer[0]);

            // Build polygons
            GEO_PolyCounts polyCounts;
            for (size_t i = 0; i < faceCounts.size(); ++i)
            {
                polyCounts.append(faceCounts[i]);
            }

            GU_PrimPoly::buildBlock(gdp, ptoff, nbPoints, polyCounts, &elementsIndices[0]);

            // Add group
            std::string groupName = "mesh_" + boost::lexical_cast<std::string>(m_groupIndex);
            m_groupIndex++;
            GA_PrimitiveGroup* primGrp = gdp->newPrimitiveGroup(groupName.c_str());
            GA_Range primRange = gdp->getPrimitiveRangeSlice(GA_Index(gdp->getNumPrimitives() - faceCounts.size()),
                                                             GA_Index(gdp->getNumPrimitives()));
            primGrp->addRange(primRange);
        }

        select(GU_SPrimitive);
    }
}

OP_ERROR SOP_FabricGenerator::cookMySop(OP_Context& context)
{
    // Start the interrupt server
    UT_AutoInterrupt boss("Evaluating FabricDFG");
    if (boss.wasInterrupted())
    {
        return error();
    }

    try
    {
        fpreal now = context.getTime();
        updateGraph(now);
        executeGraph();

        gdp->clearAndDestroy();

        m_groupIndex = 0;
        FabricCore::Client client;
        FabricCore::DFGBinding binding;
        std::vector<FTL::CStrRef> outputPortNames;
        getView().getPolygonMeshOutputPorts(client, binding, outputPortNames);
        BOOST_FOREACH (const FTL::CStrRef& portName, outputPortNames)
        {
            FabricCore::RTVal rtVal = binding.getArgValue(portName.c_str());
            setPolygonMesh(rtVal);
        }
    }
    catch (FabricCore::Exception e)
    {
        printf("FabricCore::Exception from SOP_FabricGenerator::cookMySop:\n %s\n", e.getDesc_cstr());
        return error();
    }

    return error();
}
