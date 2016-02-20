// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "MultiParams.h"
#include <PRM/PRM_Include.h>
#include <SOP/SOP_Node.h>

#define INIT_MULTI_PARAMETER_TYPE(TOKEN, PRM_Type, floatDefault, stringDefault)                                        \
    PRM_Name MultiParams::TOKEN##Ports(#TOKEN "Ports", #TOKEN " Ports");                                               \
    PRM_Name MultiParams::TOKEN##Port(#TOKEN "Port#", "");                                                             \
    PRM_Default MultiParams::TOKEN##PortDefault(0, "name1");                                                           \
    PRM_Name MultiParams::TOKEN##PortVal(#TOKEN "PortVal#", "");                                                       \
    PRM_Name MultiParams::TOKEN##PortxVal(#TOKEN "PortxVal#", "");                                                     \
    PRM_Name MultiParams::TOKEN##PortyVal(#TOKEN "PortyVal#", "");                                                     \
    PRM_Name MultiParams::TOKEN##PortzVal(#TOKEN "PortzVal#", "");                                                     \
    PRM_Default MultiParams::TOKEN##PortValDefault(floatDefault, stringDefault)

#define ADD_PARM1_TEMPLATE(TOKEN, PRM_Type, RANGE)                                                                     \
    PRM_Template MultiParams::TOKEN##PortsTemplate[] = {                                                               \
        PRM_Template(PRM_ALPHASTRING | PRM_TYPE_LABEL | PRM_TYPE_JOIN_NEXT | PRM_TYPE_NOCOOK,                          \
                     1,                                                                                                \
                     &TOKEN##Port,                                                                                     \
                     &portDefaultName),                                                                                \
        PRM_Template(PRM_Type, 1, &TOKEN##PortVal, &TOKEN##PortValDefault, 0, RANGE),                                  \
        PRM_Template()                                                                                                 \
    }

// Can't use XYZ with vector size of 3 (crash when used with a string as first template)
// So we create our own Vec3 param from three float params.
#define ADD_PARM3_TEMPLATE(TOKEN, PRM_Type, RANGE)                                                                     \
    PRM_Template MultiParams::TOKEN##PortsTemplate[] = {                                                               \
        PRM_Template(PRM_ALPHASTRING | PRM_TYPE_LABEL | PRM_TYPE_JOIN_NEXT | PRM_TYPE_NOCOOK,                          \
                     1,                                                                                                \
                     &TOKEN##Port,                                                                                     \
                     &portDefaultName),                                                                                \
        PRM_Template(PRM_Type | PRM_TYPE_JOIN_NEXT, 1, &TOKEN##PortxVal, &TOKEN##PortValDefault, 0, RANGE),            \
        PRM_Template(PRM_Type | PRM_TYPE_JOIN_NEXT, 1, &TOKEN##PortyVal, &TOKEN##PortValDefault, 0, RANGE),            \
        PRM_Template(PRM_Type, 1, &TOKEN##PortzVal, &TOKEN##PortValDefault),                                           \
        PRM_Template()                                                                                                 \
    }

#define ADD_MULTIPARM_TEMPLATE(TOKEN)                                                                                  \
    PRM_Template MultiParams::TOKEN##PortsMultiTemplate(PRM_MULTITYPE_NO_CONTROL_UI,                                   \
                                                        TOKEN##PortsTemplate,                                          \
                                                        0,                                                             \
                                                        &TOKEN##Ports,                                                 \
                                                        PRMzeroDefaults,                                               \
                                                        0,                                                             \
                                                        &PRM_SpareData::multiStartOffsetZero)

#define BUILD_1_PARAMETER(TOKEN, PRM_Type, RANGE, floatDefault, stringDefault)                                         \
    INIT_MULTI_PARAMETER_TYPE(TOKEN, PRM_Type, floatDefault, stringDefault);                                           \
    ADD_PARM1_TEMPLATE(TOKEN, PRM_Type, RANGE);                                                                        \
    ADD_MULTIPARM_TEMPLATE(TOKEN)

#define BUILD_3_PARAMETERS(TOKEN, PRM_Type, RANGE, floatDefault, stringDefault)                                        \
    INIT_MULTI_PARAMETER_TYPE(TOKEN, PRM_Type, floatDefault, stringDefault);                                           \
    ADD_PARM3_TEMPLATE(TOKEN, PRM_Type, RANGE);                                                                        \
    ADD_MULTIPARM_TEMPLATE(TOKEN)

// PRM_MULTITYPE_LIST
// PRM_MULTITYPE_NO_CONTROL_UI
namespace OpenSpliceHoudini
{

MultiParams::MultiParams()
{
}

MultiParams::~MultiParams()
{
}

PRM_Default MultiParams::portDefaultName(0, "name1");
PRM_Range MultiParams::indexRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 100);

BUILD_1_PARAMETER(Float32, PRM_FLT, 0, 0.0, "");
BUILD_1_PARAMETER(SInt32, PRM_INT, 0, 0.0, "");
BUILD_1_PARAMETER(UInt32, PRM_INT, &indexRange, 0.0, "");
BUILD_1_PARAMETER(Index, PRM_INT, &indexRange, 0.0, "");
BUILD_1_PARAMETER(Size, PRM_INT, &indexRange, 0.0, "");
BUILD_1_PARAMETER(Count, PRM_INT, &indexRange, 0.0, "");
BUILD_1_PARAMETER(String, PRM_STRING, 0, 0.0, "");
BUILD_1_PARAMETER(FilePath, PRM_FILE, 0, 0.0, "");
BUILD_3_PARAMETERS(Vec3, PRM_FLT, 0, 0.0, "");

int MultiParams::addInstance(OP_Parameters* op, const std::string& multiParmName, const std::string& name)
{
    int numInstances = op->evalFloat(multiParmName.c_str(), 0, 0);
    std::string nameInstParm = multiParmName.substr(0, multiParmName.length() - 1) + "#";
    op->insertMultiParmItem(multiParmName.c_str(), numInstances + 1);
    int startIdx = op->getParm(multiParmName.c_str()).getMultiStartOffset();
    int instance_idx = startIdx + numInstances;
    op->setStringInst(name.c_str(), CH_STRING_LITERAL, nameInstParm.c_str(), &instance_idx, 0, 0);
    return instance_idx;
}

bool MultiParams::removeInstance(OP_Parameters* op, const std::string& multiParmTypeName, const std::string& name)
// preconditions: multiParmTypeName is always a supported KL type
{
    const std::string multiParmName = multiParmTypeName + "Ports";
    const std::string instanceName = multiParmTypeName + "Port#";
    int num_param_instances = op->evalFloat(multiParmName.c_str(), 0, 0);
    int instance_idx = op->getParm(multiParmName.c_str()).getMultiStartOffset();
    for (int i = 0; i < num_param_instances; ++i)
    {
        UT_String currentName;
        op->evalStringInst(instanceName.c_str(), &i, currentName, 0, 0);
        if (currentName == UT_String(name.c_str()))
        {
            op->removeMultiParmItem(multiParmName.c_str(), i);
            return true;
        }
        instance_idx++;
    }
    return false;
}

bool MultiParams::removeInstance(OP_Parameters* op, const std::string& name)
{
    bool removed = MultiParams::removeInstance(op, "Float32", name);

    if (!removed)
    {
        removed = MultiParams::removeInstance(op, "SInt32", name);
    }
    if (!removed)
    {
        removed = MultiParams::removeInstance(op, "UInt32", name);
    }
    if (!removed)
    {
        removed = MultiParams::removeInstance(op, "String", name);
    }
    if (!removed)
    {
        removed = MultiParams::removeInstance(op, "FilePath", name);
    }
    if (!removed)
    {
        removed = MultiParams::removeInstance(op, "Vec3", name);
    }
    return removed;
}

void MultiParams::renameInstance(OP_Parameters* op,
                                 const std::string& multiParmTypeName,
                                 const std::string& oldName,
                                 const std::string& newName)
{
    const std::string multiParmName = multiParmTypeName + "Ports";
    const std::string instanceName = multiParmTypeName + "Port#";

    int num_param_instances = op->evalFloat(multiParmName.c_str(), 0, 0);
    int instance_idx = op->getParm(multiParmName.c_str()).getMultiStartOffset();

    for (int i = 0; i < num_param_instances; ++i)
    {
        UT_String name;
        op->evalStringInst(instanceName.c_str(), &instance_idx, name, 0, 0);
        if (name == UT_String(oldName.c_str()))
        {
            op->setStringInst(UT_String(newName), CH_STRING_LITERAL, instanceName.c_str(), &instance_idx, 0, 0);
            break;
        }
        instance_idx++;
    }
}

void MultiParams::addFloatParameterInst(OP_Parameters* op, const std::string& name, float val)
{
    std::string multiParmName("Float32Ports");
    int instance_idx = MultiParams::addInstance(op, multiParmName, name);
    std::string valueParm = multiParmName.substr(0, multiParmName.length() - 1) + "Val#";
    op->setFloatInst(val, valueParm.c_str(), &instance_idx, 0, 0);
}

const UT_String MultiParams::getParameterInstFloatName(OP_Parameters* op, int instance_idx)
{
    UT_String name;
    op->evalStringInst("Float32Port#", &instance_idx, name, 0, 0);
    return name;
}

float MultiParams::getParameterInstFloatValue(OP_Parameters* op, int instance_idx, fpreal t)
{
    return op->evalFloatInst("Float32PortVal#", &instance_idx, 0, t);
}

void MultiParams::addIntParameterInst(OP_Parameters* op, const std::string& name, int val, const std::string& option)
{
    std::string multiParmName = option + "Ports";

    int instance_idx = MultiParams::addInstance(op, multiParmName, name);
    std::string valueParm = multiParmName.substr(0, multiParmName.length() - 1) + "Val#";
    op->setIntInst(val, valueParm.c_str(), &instance_idx, 0, 0);
}

const UT_String MultiParams::getParameterInstIntName(OP_Parameters* op, int instance_idx, const std::string& option)
{
    std::string instanceName = option + "Port#";

    UT_String name;
    op->evalStringInst(instanceName.c_str(), &instance_idx, name, 0, 0);
    return name;
}

int MultiParams::getParameterInstIntValue(OP_Parameters* op, int instance_idx, const std::string& option, fpreal t)
{
    std::string instanceValName = option + "PortVal#";
    return op->evalIntInst(instanceValName.c_str(), &instance_idx, 0, t);
}

void MultiParams::addStringParameterInst(OP_Parameters* op,
                                         const std::string& name,
                                         const char* val,
                                         const std::string& option)
{
    std::string multiParmName = option + "Ports";
    int instance_idx = MultiParams::addInstance(op, multiParmName, name);
    std::string valueParm = multiParmName.substr(0, multiParmName.length() - 1) + "Val#";
    op->setStringInst(UT_String(val), CH_STRING_LITERAL, valueParm.c_str(), &instance_idx, 0, 0);
}

const UT_String MultiParams::getParameterInstStringName(OP_Parameters* op, int instance_idx, const std::string& option)
{
    std::string parmName = option + "Port#";

    UT_String name;
    op->evalStringInst(parmName.c_str(), &instance_idx, name, 0, 0);
    return name;
}

const UT_String
MultiParams::getParameterInstStringValue(OP_Parameters* op, int instance_idx, fpreal t, const std::string& option)
{
    std::string parmName = option + "PortVal#";
    UT_String val;
    op->evalStringInst(parmName.c_str(), &instance_idx, val, 0, t);
    return val;
}

void MultiParams::addVec3ParameterInst(OP_Parameters* op, const std::string& name, Imath::Vec3<float> val)
{
    std::string multiParmName("Vec3Ports");
    int instance_idx = MultiParams::addInstance(op, multiParmName, name);
    std::string valueParmX = multiParmName.substr(0, multiParmName.length() - 1) + "xVal#";
    std::string valueParmY = multiParmName.substr(0, multiParmName.length() - 1) + "yVal#";
    std::string valueParmZ = multiParmName.substr(0, multiParmName.length() - 1) + "zVal#";
    op->setFloatInst(val.x, valueParmX.c_str(), &instance_idx, 0, 0);
    op->setFloatInst(val.y, valueParmY.c_str(), &instance_idx, 1, 0);
    op->setFloatInst(val.z, valueParmZ.c_str(), &instance_idx, 2, 0);
}

const UT_String MultiParams::getParameterInstVec3Name(OP_Parameters* op, int instance_idx)
{
    UT_String name;
    op->evalStringInst("Vec3Port#", &instance_idx, name, 0, 0);
    return name;
}

Imath::Vec3<float> MultiParams::getParameterInstVec3Value(OP_Parameters* op, int instance_idx, fpreal t)
{
    Imath::Vec3<float> val;
    val.x = op->evalFloatInst("Vec3PortxVal#", &instance_idx, 0, t);
    val.y = op->evalFloatInst("Vec3PortyVal#", &instance_idx, 0, t);
    val.z = op->evalFloatInst("Vec3PortzVal#", &instance_idx, 0, t);
    return val;
}

} // End namespace OpenSpliceHoudini
