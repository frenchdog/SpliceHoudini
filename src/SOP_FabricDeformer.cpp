// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "SOP_FabricDeformer.h"
#include "core/AttributeTraits.h"

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <CH/CH_LocalVariable.h>
#include <UT/UT_Interrupt.h>
#include <SYS/SYS_Math.h>
#include <limits.h>
#include <GU/GU_PrimPoly.h>
#include <GEO/GEO_PolyCounts.h>
#include <OP/OP_AutoLockInputs.h>

#include <ImathVec.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;

static PRM_Name pointAttributesName("pointAttributes", "Points Attributes");
static PRM_Template pointAttributesTemplate(PRM_STRING, 1, &pointAttributesName);

namespace
{

std::vector<std::string> getAttributeNameTokens(SOP_FabricDeformer& sop)
{
    std::string paramVal = sop.getStringValue("pointAttributes", 0).toStdString();
    std::vector<std::string> result;
    boost::split(result, paramVal, boost::is_any_of(",; "));
    return result;
}

} // end unnamed namespace

OP_TemplatePair* SOP_FabricDeformer::buildTemplatePair(OP_TemplatePair* prevstuff)
{
    static PRM_Template* theTemplate = 0;
    theTemplate = new PRM_Template[2];

    theTemplate[0] = pointAttributesTemplate;
    theTemplate[1] = PRM_Template();

    OP_TemplatePair* dfg, *geo;
    dfg = new OP_TemplatePair(myTemplateList, prevstuff);
    geo = new OP_TemplatePair(theTemplate, dfg);

    return geo;
}

OP_Node* SOP_FabricDeformer::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_FabricDeformer(net, name, op);
}

SOP_FabricDeformer::SOP_FabricDeformer(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<SOP_Node>(net, name, op)
{
    mySopFlags.setManagesDataIDs(true);
}

SOP_FabricDeformer::~SOP_FabricDeformer()
{
}

FabricCore::RTVal SOP_FabricDeformer::CreatePolygonMeshRTVal(const GU_Detail& gdpRef, SOP_FabricDeformer& sop)
{
    FabricCore::Client client = sop.getView().getClient();
    FabricCore::RTVal polygonMesh = FabricCore::RTVal::Create(client, "PolygonMesh", 0, 0);

    FabricCore::RTVal geometryWrapper = FabricCore::RTVal::Create(client, "SpliceHoudiniGeometryWrapper", 0, 0);
    try
    {
        std::vector<FabricCore::RTVal> args(1);
        args[0] = FabricCore::RTVal::ConstructString(client, "PolygonMesh");
        geometryWrapper.callMethod("", "setPolygonMesh", 1, &polygonMesh);
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[SOP_FabricDeformer::CreatePolygonMeshRTVal]: ") + e.getDesc_cstr()).c_str());
    }

    // Setting P attribute is required before adding other point attributes
    GA_ROHandleV3 handle(gdpRef.findAttribute(GA_ATTRIB_POINT, "P"));
    if (!handle.isValid())
        return polygonMesh;

    size_t bufferSize = static_cast<size_t>(gdpRef.getNumPoints());
    std::vector<UT_Vector3F> posBuffer(bufferSize);
    handle.getBlock(GA_Offset(), gdpRef.getNumPoints(), &posBuffer[0]);

    try
    {
        std::vector<FabricCore::RTVal> args(1);
        args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Vec3", bufferSize, &posBuffer[0]);
        geometryWrapper.callMethod("", "setP", 1, &args[0]);
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[SOP_FabricDeformer::CreatePolygonMeshRTVal]: ") + e.getDesc_cstr()).c_str());
    }

    // Get other per-point attributes
    BOOST_FOREACH (const std::string& attrName, getAttributeNameTokens(sop))
    {
        if (attrName == "P")
            continue;

        const GA_Attribute* attrib = gdpRef.findAttribute(GA_ATTRIB_POINT, attrName.c_str());
        if (attrib == 0)
            continue;

        if (attrib->getTupleSize() == 1)
        {
            if (attrib->getStorageClass() == GA_STORECLASS_INT)
            {
                HouToFabAttributeTraits<int32>::setAttribute(GA_ROHandleI(attrib),
                                                             attrib->getTypeInfo(),
                                                             gdpRef.getNumPoints(),
                                                             client,
                                                             geometryWrapper,
                                                             attrName.c_str());
            }
            if (attrib->getStorageClass() == GA_STORECLASS_REAL)
            {
                HouToFabAttributeTraits<fpreal32>::setAttribute(GA_ROHandleF(attrib),
                                                                attrib->getTypeInfo(),
                                                                gdpRef.getNumPoints(),
                                                                client,
                                                                geometryWrapper,
                                                                attrName.c_str());
            }
        }
        else if (attrib->getTupleSize() == 3)
        {
            HouToFabAttributeTraits<UT_Vector3F>::setAttribute(GA_ROHandleV3(attrib),
                                                               attrib->getTypeInfo(),
                                                               gdpRef.getNumPoints(),
                                                               client,
                                                               geometryWrapper,
                                                               attrName.c_str());
        }
        else if (attrib->getTupleSize() == 4)
        {
            HouToFabAttributeTraits<UT_Vector4F>::setAttribute(GA_ROHandleV4(attrib),
                                                               attrib->getTypeInfo(),
                                                               gdpRef.getNumPoints(),
                                                               client,
                                                               geometryWrapper,
                                                               attrName.c_str());
        }
    }

    return polygonMesh;
}

void SOP_FabricDeformer::OnUpdateGraphCopyAttributes(OP_Network& node, FabricCore::DFGBinding& binding)
{
    SOP_FabricDeformer& sop = static_cast<SOP_FabricDeformer&>(node);
    GU_Detail& gdpRef = *(sop.gdp);

    FabricCore::DFGExec exec = binding.getExec();
    uint32_t portCount = exec.getExecPortCount();

    // Set first PolygonMesh input port
    for (uint32_t i = 0; i < portCount; ++i)
    {
        if (exec.getExecPortType(i) == FabricCore::DFGPortType_In)
        {
            FTL::CStrRef name = exec.getExecPortName(i);
            FTL::CStrRef resolvedType = exec.getExecPortResolvedType(i);

            if (resolvedType == FTL_STR("PolygonMesh"))
            {
                FabricCore::RTVal polygonMesh = CreatePolygonMeshRTVal(gdpRef, sop);
                binding.setArgValue(name.c_str(), polygonMesh);
                break;
            }
        }
    }

    // Set array ports using attributes from second input
    const GU_Detail* gdp2 = sop.inputGeo(1);
    if (gdp2)
    {
        size_t bufferSize = gdp2->getNumPoints();

        for (uint32_t i = 0; i < portCount; ++i)
        {
            if (exec.getExecPortType(i) == FabricCore::DFGPortType_In)
            {
                FTL::CStrRef name = exec.getExecPortName(i);
                const GA_Attribute* attrib = gdp2->findAttribute(GA_ATTRIB_POINT, name.c_str());

                if (attrib && attrib->getTupleSize() == 1 && attrib->getStorageClass() == GA_STORECLASS_INT)
                {
                    sop.addIntegerBuffer(bufferSize);
                    GA_ROHandleI handle(attrib);
                    if (!handle.isValid())
                        continue;
                    handle.getBlock(GA_Offset(), gdp2->getNumPoints(), sop.getIntegerBuffer());
                    try
                    {
                        FabricCore::RTVal extArrayValue;
                        FabricCore::Client client = sop.getView().getClient();

                        extArrayValue = FabricCore::RTVal::ConstructExternalArray(
                            client, "SInt32", bufferSize, sop.getIntegerBuffer());
                        binding.setArgValue(name.c_str(), extArrayValue);
                    }
                    catch (FabricCore::Exception e)
                    {
                        FabricCore::Exception::Throw("External Array not contructed");
                    }
                }
                if (attrib && attrib->getTupleSize() == 1 && attrib->getStorageClass() == GA_STORECLASS_REAL)
                {
                    sop.addFloatBuffer(bufferSize);
                    GA_ROHandleF handle(attrib);
                    if (!handle.isValid())
                        continue;
                    handle.getBlock(GA_Offset(), gdp2->getNumPoints(), sop.getFloatBuffer());
                    try
                    {
                        FabricCore::RTVal extArrayValue;
                        FabricCore::Client client = sop.getView().getClient();

                        extArrayValue = FabricCore::RTVal::ConstructExternalArray(
                            client, "Float32", bufferSize, sop.getFloatBuffer());
                        binding.setArgValue(name.c_str(), extArrayValue);
                    }
                    catch (FabricCore::Exception e)
                    {
                        FabricCore::Exception::Throw("External Array not contructed");
                    }
                }
                else if (attrib && (attrib->getTupleSize() == 3 || attrib->getTupleSize() == 4))
                {
                    sop.addVec3Buffer(bufferSize);
                    GA_ROHandleV3 handle(attrib);
                    if (!handle.isValid())
                        continue;
                    handle.getBlock(GA_Offset(), gdp2->getNumPoints(), sop.getVec3Buffer());
                    try
                    {
                        FabricCore::RTVal extArrayValue;
                        FabricCore::Client client = sop.getView().getClient();

                        extArrayValue = FabricCore::RTVal::ConstructExternalArray(
                            client,
                            attrib->getTypeInfo() == GA_TYPE_COLOR ? "Color" : "Vec3",
                            bufferSize,
                            sop.getVec3Buffer());
                        binding.setArgValue(name.c_str(), extArrayValue);
                    }
                    catch (FabricCore::Exception e)
                    {
                        FabricCore::Exception::Throw("External Array not contructed");
                    }
                }
            }
        }
    }
}

void SOP_FabricDeformer::setPointsPositions(OP_Context& context)
{
    FabricCore::Client client;
    FabricCore::DFGBinding binding;
    std::vector<FTL::CStrRef> outputPortNames;
    getView().getPolygonMeshOutputPorts(client, binding, outputPortNames);
    if (outputPortNames.size() > 0)
    {
        FTL::CStrRef portName = outputPortNames[0];

        FabricCore::RTVal polygonMesh = binding.getArgValue(portName.c_str());

        size_t nbPoints = 0;
        if (polygonMesh.isValid() && !polygonMesh.isNullObject())
        {
            try
            {
                FabricCore::RTVal rtValPointCount;
                rtValPointCount = polygonMesh.callMethod("Size", "pointCount", 0, 0);
                nbPoints = rtValPointCount.getUInt32();
            }
            catch (FabricCore::Exception e)
            {
                (std::string("[SOP_FabricDeformer::setPointsPositions]: ") + e.getDesc_cstr()).c_str();
            }

            size_t inPtsCount = static_cast<size_t>(gdp->getNumPoints());
            if (nbPoints != inPtsCount)
            {
                std::cout << "Point Count Mismatch ! gdp is: " << inPtsCount << " and output port '" << portName
                          << "' is: " << nbPoints << std::endl;
                return;
            }

            std::vector<UT_Vector3F> posBuffer(nbPoints);
            try
            {
                std::vector<FabricCore::RTVal> args(2);
                args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Float32", nbPoints * 3, &posBuffer[0]);
                args[1] = FabricCore::RTVal::ConstructUInt32(client, 3); // components

                polygonMesh.callMethod("", "getPointsAsExternalArray", 2, &args[0]);
            }
            catch (FabricCore::Exception e)
            {
                (std::string("[SOP_FabricDeformer::setPointsPositions]: ") + e.getDesc_cstr()).c_str();
            }

            GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, "P"));
            handle.setBlock(GA_Offset(), gdp->getNumPoints(), &posBuffer[0]);
            handle.bumpDataId();
        }
    }
}

OP_ERROR SOP_FabricDeformer::cookMySop(OP_Context& context)
{
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    // Duplicate input geometry
    duplicateSource(0, context);

    try
    {
        fpreal now = context.getTime();
        updateGraph(now, SOP_FabricDeformer::OnUpdateGraphCopyAttributes);
        executeGraph();
        setPointsPositions(context);
    }
    catch (FabricCore::Exception e)
    {
        std::string msg = "FabricCore::Exception from SOP_FabricDeformer::cookMySop:\n";
        msg += e.getDesc_cstr();
        std::cerr << msg << std::endl;
        addError(SOP_MESSAGE, msg.c_str());
        return error();
    }

    if (!getView().hasOuputPort())
    {
        addWarning(SOP_MESSAGE, "No Canvas output ports, Fabric Graph will not be executed");
    }

    return error();
}
