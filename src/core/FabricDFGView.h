// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __FabricDFGView_H__
#define __FabricDFGView_H__

#include "FabricDFGView.fwd.h"

#define FEC_SHARED
#define FECS_SHARED

#include <FabricCore.h>
#include <FTL/CStrRef.h>
#include <ASTWrapper/KLASTManager.h>
#include <Commands/CommandStack.h>

#include <vector>
#include <map>
#include <ImathVec.h>

class OP_Network;

namespace OpenSpliceHoudini
{

typedef void (*CopyAttributesFunc)(OP_Network& node, FabricCore::DFGBinding& binding);

/// A Fabric View with access to an Houdini node
class FabricDFGView
{

public:
    FabricDFGView(OP_Network* op);
    ~FabricDFGView();

    unsigned int getId();
    static FabricDFGView* getFromId(unsigned int id);

    static FabricCore::Client& getClient();
    static FabricCore::DFGHost& getHost();
    FabricCore::DFGBinding getBinding();
    static FabricServices::ASTWrapper::KLASTManager* getManager();
    static FabricServices::Commands::CommandStack* getStack();

    /// persistence
    std::string getJSON();
    void createBindingFromJSON(const std::string& json);

    /// logging.
    static void setLogFunc(void (*in_logFunc)(void*, const char*, unsigned int));
    static void logErrorFunc(void* userData, const char* message, unsigned int length);

    static void (*s_logFunc)(void*, const char*, unsigned int);
    static void (*s_logErrorFunc)(void*, const char*, unsigned int);

    void setMyGraph()
    {
        FabricCore::DFGExec exec = m_binding.getExec();
        m_view = exec.createView(&ViewCallback, this);
    }

    void setWidget(FabricDFGWidgetPtr widget)
    {
        m_widget = widget;
    }

    bool hasOuputPort();

protected:
    void viewCallback(FTL::CStrRef jsonStr);

    static void ViewCallback(void* userdata, char const* jsonCStr, uint32_t jsonSize)
    {
        static_cast<FabricDFGView*>(userdata)->viewCallback(FTL::CStrRef(jsonCStr, jsonSize));
    }

private:
    static void logFunc(void* userData, const char* message, unsigned int length);

    static FabricCore::Client s_client;
    static FabricCore::DFGHost s_host;
    mutable FabricCore::DFGBinding m_binding;
    mutable FabricCore::DFGView m_view;
    static FabricServices::ASTWrapper::KLASTManager* s_manager;
    static FabricServices::Commands::CommandStack s_stack;
    unsigned int m_id;
    static unsigned int s_maxId;
    static std::map<unsigned int, FabricDFGView*> s_instances;

    void setSInt32PortValue(const char* name, int val);
    void setUInt32PortValue(const char* name, int val);
    void setFloat32PortValue(const char* name, float val);
    void setStringPortValue(const char* name, const char* val);
    void setFilePathPortValue(const char* name, const char* val);
    void setVec3PortValue(const char* name, const Imath::Vec3<float>& val);

public:
    /// Houdini and DFG bindings
    void addParametersFromInputPorts();
    void getPolygonMeshOutputPorts(FabricCore::Client& client,
                                   FabricCore::DFGBinding& binding,
                                   std::vector<FTL::CStrRef>& outputPortNames) const;

    /// Set Canvas input ports using the Houdini node data (params and/or attributes)
    void setInputPortsFromOpNode(const float t, CopyAttributesFunc func);

private:
    void dirtyOp(bool saveGraph);
    void saveJsonData();

    FabricDFGWidgetPtr m_widget;
    OP_Network* m_op;
};
} // End namespace OpenSpliceHoudini

#endif
