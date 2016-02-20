// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "FabricDFGView.h"
#include "MultiParams.h"
#include "ParameterFactory.h"

#include <OP/OP_Network.h>
#include <FTL/JSONValue.h>

using namespace FabricServices;

namespace OpenSpliceHoudini
{

FabricCore::Client FabricDFGView::s_client;
FabricCore::DFGHost FabricDFGView::s_host;
FabricServices::ASTWrapper::KLASTManager* FabricDFGView::s_manager = NULL;
FabricServices::Commands::CommandStack FabricDFGView::s_stack;
unsigned int FabricDFGView::s_maxId = 0;
void (*FabricDFGView::s_logFunc)(void*, const char*, unsigned int) = NULL;
void (*FabricDFGView::s_logErrorFunc)(void*, const char*, unsigned int) = NULL;

std::map<unsigned int, FabricDFGView*> FabricDFGView::s_instances;

FabricDFGView::FabricDFGView(OP_Network* op)
    : m_op(op)
{
    m_id = s_maxId++;

    // construct the client
    if (s_instances.size() == 0)
    {
        try
        {
            // create a client
            FabricCore::Client::CreateOptions options;
            memset(&options, 0, sizeof(options));
            options.optimizationType = FabricCore::ClientOptimizationType_Background;
            options.guarded = 1;
            s_client = FabricCore::Client(&logFunc, NULL, &options);

            // Load basic extensions
            s_client.loadExtension("Math", "", false);
            s_client.loadExtension("Geometry", "", false);
            s_client.loadExtension("FileIO", "", false);
            s_client.loadExtension("SpliceHoudini", "", false);

            // create a host for Canvas
            s_host = s_client.getDFGHost();

            // create KL AST manager
            s_manager = new ASTWrapper::KLASTManager(&s_client);
        }
        catch (FabricCore::Exception e)
        {
            printf("Error: %s\n", e.getDesc_cstr());
        }
    }

    s_instances.insert(std::pair<unsigned int, FabricDFGView*>(m_id, this));

    // create an empty binding
    m_binding = s_host.createBindingToNewGraph();

    setMyGraph();

    ParameterFactory::RegisterTypes();
}

FabricDFGView::~FabricDFGView()
{
    std::map<unsigned int, FabricDFGView*>::iterator it = s_instances.find(m_id);

    m_view.invalidate();
    m_binding.invalidate();

    if (it != s_instances.end())
    {
        s_instances.erase(it);
        if (s_instances.size() == 0)
        {
            try
            {
                s_stack.clear();
                delete (s_manager);
                s_host.invalidate();
                s_client = FabricCore::Client();
            }
            catch (FabricCore::Exception e)
            {
                printf("Error: %s\n", e.getDesc_cstr());
            }
        }
    }
}

unsigned int FabricDFGView::getId()
{
    return m_id;
}

FabricDFGView* FabricDFGView::getFromId(unsigned int id)
{
    std::map<unsigned int, FabricDFGView*>::iterator it = s_instances.find(id);
    if (it == s_instances.end())
        return NULL;
    return it->second;
}

FabricCore::Client& FabricDFGView::getClient()
{
    return s_client;
}

FabricCore::DFGHost& FabricDFGView::getHost()
{
    return s_host;
}

FabricCore::DFGBinding FabricDFGView::getBinding()
{
    return m_binding;
}

FabricServices::ASTWrapper::KLASTManager* FabricDFGView::getManager()
{
    return s_manager;
}

FabricServices::Commands::CommandStack* FabricDFGView::getStack()
{
    return &s_stack;
}

std::string FabricDFGView::getJSON()
{
    try
    {
        return m_binding.exportJSON().getCString();
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[FabricDFGView::createBindingFromJSON: ") + e.getDesc_cstr()).c_str());
    }
    return "";
}

void FabricDFGView::createBindingFromJSON(const std::string& json)
{
    try
    {
        m_binding = s_host.createBindingFromJSON(json.c_str());
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[FabricDFGView::createBindingFromJSON: ") + e.getDesc_cstr()).c_str());
    }
}

void FabricDFGView::setLogFunc(void (*in_logFunc)(void*, const char*, unsigned int))
{
    s_logFunc = in_logFunc;
}

void FabricDFGView::viewCallback(FTL::CStrRef jsonStr)
{
    // printf("FabricDFGView: notif: %s\n", jsonStr.c_str());

    FTL::JSONStrWithLoc jsonStrWithLoc(jsonStr);
    FTL::OwnedPtr<FTL::JSONValue const> jsonValue(FTL::JSONValue::Decode(jsonStrWithLoc));
    if (!jsonValue)
        return;
    FTL::JSONObject const* jsonObject = jsonValue->maybeCast<FTL::JSONObject>();
    if (!jsonObject)
        return;
    FTL::CStrRef descStr;
    if (!jsonObject->maybeGetString(FTL_STR("desc"), descStr))
        return;

    if (descStr == FTL_STR("execPortInserted") || descStr == FTL_STR("portsConnected") ||
        descStr == FTL_STR("portsDisconnected"))
        dirtyOp(true);
    else if (descStr == FTL_STR("execPortRemoved"))
    {
        FTL::CStrRef portName;
        if (jsonObject->maybeGetString(FTL_STR("portName"), portName))
        {
            if (MultiParams::removeInstance(m_op, portName.c_str()))
                saveJsonData();
        }
    }
    else if (descStr == FTL_STR("execPortRenamed"))
    {
        FTL::CStrRef oldPortName;
        if (jsonObject->maybeGetString(FTL_STR("oldPortName"), oldPortName))
        {
            FTL::CStrRef newPortName;
            if (jsonObject->maybeGetString(FTL_STR("newPortName"), newPortName))
            {
                FTL::CStrRef resolvedType = m_binding.getExec().getExecPortResolvedType(newPortName.c_str());
                if (MultiParams::isSupportedType(resolvedType.c_str()))
                {
                    MultiParams::renameInstance(m_op, resolvedType.c_str(), oldPortName.c_str(), newPortName.c_str());
                    saveJsonData();
                    dirtyOp(true);
                }
            }
        }
    }
    else if (descStr == FTL_STR("execPortResolvedTypeChanged"))
    {
        FTL::CStrRef portName;
        if (jsonObject->maybeGetString(FTL_STR("portName"), portName))
        {
            FTL::CStrRef newResolvedType;
            if (jsonObject->maybeGetString(FTL_STR("newResolvedType"), newResolvedType))
            {
                switch (m_binding.getExec().getExecPortType(portName.c_str()))
                {
                case FabricCore::DFGPortType_In:
                {
                    ParameterFactory::CreateParameterFunc addParam = ParameterFactory::Get(newResolvedType.c_str());
                    if (addParam)
                    {
                        // remove previous multi-parm instance using this name.
                        MultiParams::removeInstance(m_op, portName.c_str());
                        addParam(m_op, portName.c_str());
                    }
                }
                break;

                case FabricCore::DFGPortType_IO:
                    dirtyOp(true);
                    break;

                case FabricCore::DFGPortType_Out:
                    if (newResolvedType == FTL_STR("PolygonMesh") || newResolvedType == FTL_STR("Vec3"))
                    {
                        dirtyOp(true);
                    }
                    break;
                }
            }
        }
    }
}

void FabricDFGView::addParametersFromInputPorts()
{
    FabricCore::DFGExec exec = m_binding.getExec();
    uint32_t portCount = exec.getExecPortCount();
    for (uint32_t i = 0; i < portCount; ++i)
    {
        if (exec.getExecPortType(i) == FabricCore::DFGPortType_In)
        {
            FTL::CStrRef portName = exec.getExecPortName(i);
            FTL::CStrRef resolvedType = exec.getExecPortResolvedType(i);
            ParameterFactory::CreateParameterFunc addParam = ParameterFactory::Get(resolvedType.c_str());
            if (addParam)
            {
                addParam(m_op, portName.c_str());
            }
            else
            {
                std::cout << "FabricDFGView::onPortResolvedTypeChanged: " << portName
                          << " is a Canvas only input ! Type " << resolvedType << " not reflected by Houdini"
                          << std::endl;
            }
        }
    }
}

bool FabricDFGView::hasOuputPort()
{
    FabricCore::DFGExec exec = m_binding.getExec();
    uint32_t portCount = exec.getExecPortCount();
    for (uint32_t i = 0; i < portCount; ++i)
    {
        if (exec.getExecPortType(i) == FabricCore::DFGPortType_Out)
            return true;
    }
    return false;
}

void FabricDFGView::dirtyOp(bool saveGraph)
{
    int val = m_op->evalInt("__portsChanged", 0, 0);
    m_op->setInt("__portsChanged", 0, 0, (val + 1) % INT_MAX);
    if (saveGraph)
        saveJsonData();
}

void FabricDFGView::saveJsonData()
{
    m_op->setString(UT_String(getJSON().c_str()), CH_STRING_LITERAL, "jsonData", 0, 0);
}

void FabricDFGView::setSInt32PortValue(const char* name, int val)
{
    FabricCore::DFGExec exec = m_binding.getExec();
    if (exec.haveExecPort(name))
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructSInt32(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setUInt32PortValue(const char* name, int val)
{
    FabricCore::DFGExec exec = m_binding.getExec();
    if (exec.haveExecPort(name))
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructUInt32(s_client, static_cast<size_t>(val));
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setFloat32PortValue(const char* name, float val)
{
    FabricCore::DFGExec exec = m_binding.getExec();
    if (exec.haveExecPort(name))
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructFloat32(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setStringPortValue(const char* name, const char* val)
{
    FabricCore::DFGExec exec = m_binding.getExec();
    if (exec.haveExecPort(name))
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructString(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setFilePathPortValue(const char* name, const char* val)
{
    FabricCore::DFGExec exec = m_binding.getExec();
    if (exec.haveExecPort(name))
    {
        FabricCore::RTVal rtString = FabricCore::RTVal::ConstructString(s_client, val);
        FabricCore::RTVal rtVal = FabricCore::RTVal::Create(s_client, "FilePath", 1, &rtString);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setVec3PortValue(const char* name, const Imath::Vec3<float>& val)
{
    FabricCore::DFGExec exec = m_binding.getExec();
    if (exec.haveExecPort(name))
    {
        FabricCore::RTVal rtVec[3];
        rtVec[0] = FabricCore::RTVal::ConstructFloat32(s_client, val.x);
        rtVec[1] = FabricCore::RTVal::ConstructFloat32(s_client, val.y);
        rtVec[2] = FabricCore::RTVal::ConstructFloat32(s_client, val.z);
        FabricCore::RTVal rtVal = FabricCore::RTVal::Construct(s_client, "Vec3", 3, rtVec);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::getPolygonMeshOutputPorts(FabricCore::Client& client,
                                              FabricCore::DFGBinding& binding,
                                              std::vector<FTL::CStrRef>& outputPortNames) const
{
    client = s_client;
    binding = m_binding;
    FabricCore::DFGExec exec = binding.getExec();
    uint32_t portCount = exec.getExecPortCount();

    for (uint32_t i = 0; i < portCount; ++i)
    {
        if (exec.getExecPortType(i) == FabricCore::DFGPortType_Out)
        {
            FTL::CStrRef resolvedType = exec.getExecPortResolvedType(i);
            if (resolvedType == FTL_STR("PolygonMesh") || resolvedType == FTL_STR("PolygonMesh[]"))
            {
                outputPortNames.push_back(exec.getExecPortName(i));
            }
        }
    }
}

void FabricDFGView::setInputPortsFromOpNode(const float t, CopyAttributesFunc func)
{
    if (func)
    {
        func(*m_op, m_binding);
    }

    // Set DFG inputs ports from Houdini inputs multi-parameters
    int num_param_instances = m_op->evalFloat("Float32Ports", 0, t);
    int instance_idx = m_op->getParm("Float32Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setFloat32PortValue(MultiParams::getParameterInstFloatName(m_op, instance_idx),
                            MultiParams::getParameterInstFloatValue(m_op, instance_idx, t));

        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("SInt32Ports", 0, t);
    instance_idx = m_op->getParm("SInt32Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setSInt32PortValue(MultiParams::getParameterInstIntName(m_op, instance_idx, "SInt32"),
                           MultiParams::getParameterInstIntValue(m_op, instance_idx, "SInt32", t));

        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("UInt32Ports", 0, t);
    instance_idx = m_op->getParm("UInt32Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setUInt32PortValue(MultiParams::getParameterInstIntName(m_op, instance_idx, "UInt32"),
                           MultiParams::getParameterInstIntValue(m_op, instance_idx, "UInt32", t));

        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("IndexPorts", 0, t);
    instance_idx = m_op->getParm("IndexPorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setUInt32PortValue(MultiParams::getParameterInstIntName(m_op, instance_idx, "Index"),
                           MultiParams::getParameterInstIntValue(m_op, instance_idx, "Index", t));

        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("SizePorts", 0, t);
    instance_idx = m_op->getParm("SizePorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setUInt32PortValue(MultiParams::getParameterInstIntName(m_op, instance_idx, "Size"),
                           MultiParams::getParameterInstIntValue(m_op, instance_idx, "Size", t));

        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("CountPorts", 0, t);
    instance_idx = m_op->getParm("CountPorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setUInt32PortValue(MultiParams::getParameterInstIntName(m_op, instance_idx, "Count"),
                           MultiParams::getParameterInstIntValue(m_op, instance_idx, "Count", t));

        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("StringPorts", 0, t);
    instance_idx = m_op->getParm("StringPorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setStringPortValue(MultiParams::getParameterInstStringName(m_op, instance_idx),
                           MultiParams::getParameterInstStringValue(m_op, instance_idx, t));
        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("FilePathPorts", 0, t);
    instance_idx = m_op->getParm("FilePathPorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setFilePathPortValue(MultiParams::getParameterInstStringName(m_op, instance_idx, "FilePath"),
                             MultiParams::getParameterInstStringValue(m_op, instance_idx, t, "FilePath"));
        instance_idx++;
    }

    num_param_instances = m_op->evalFloat("Vec3Ports", 0, t);
    instance_idx = m_op->getParm("Vec3Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        setVec3PortValue(MultiParams::getParameterInstVec3Name(m_op, instance_idx),
                         MultiParams::getParameterInstVec3Value(m_op, instance_idx, t));
        instance_idx++;
    }
}

void FabricDFGView::logFunc(void* userData, const char* message, unsigned int length)
{
    if (s_logFunc)
    {
        s_logFunc(userData, message, length);
    }
    else
    {
        printf("FabricDFGView: %s\n", message);
    }
}

void FabricDFGView::logErrorFunc(void* userData, const char* message, unsigned int length)
{
    if (s_logErrorFunc)
    {
        s_logErrorFunc(userData, message, length);
    }
    else
    {
        printf("FabricDFGView: error: %s\n", message);
    }
}

} // End namespace OpenSpliceHoudini
