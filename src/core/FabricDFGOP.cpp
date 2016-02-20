// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "FabricDFGOP.h"
#include "MultiParams.h"

#include <PRM/PRM_Include.h>

#include <SOP/SOP_Node.h>
#include <OBJ/OBJ_Geometry.h>

namespace OpenSpliceHoudini
{

template <typename OP>
FabricDFGOP<OP>::FabricDFGOP(OP_Network* net, const char* name, OP_Operator* op)
    : OP(net, name, op)
    , m_graphLoaded(false)
    , m_view(this)
    , m_ui(this, m_view)
{
    OP::getParm("__portsChanged").getTemplatePtr()->setInvisible(true);
    OP::getParm("jsonData").getTemplatePtr()->setInvisible(true);
    OP::getParm("currentFrame").setExpression(0, "$F", CH_OLD_EXPR_LANGUAGE, 0);
    OP::getParm("currentFrame").setLockedFlag(0, 1);
    OP::getParm("currentFrame").getTemplatePtr()->setInvisible(true);
}

template <typename OP>
PRM_Template FabricDFGOP<OP>::groupTemplate(
    PRM_STRING, 1, &PRMgroupName, 0, &SOP_Node::pointGroupMenu, 0, 0, SOP_Node::getGroupSelectButton(GA_GROUP_POINT));

template <typename OP>
PRM_Name FabricDFGOP<OP>::jsonFilePath("jsonFilePath", "DFG File");

template <typename OP>
PRM_Default FabricDFGOP<OP>::jsonFilePathDefault(0, "$FABRIC_DIR/Samples/DFG");

template <typename OP>
PRM_Template FabricDFGOP<OP>::jsonFilePathTemplate(PRM_FILE_E, 1, &jsonFilePath, &jsonFilePathDefault);

template <typename OP>
PRM_Name FabricDFGOP<OP>::jsonData("jsonData", "JSON Data");

template <typename OP>
PRM_Default FabricDFGOP<OP>::jsonDataDefault(0, "");

template <typename OP>
PRM_Template FabricDFGOP<OP>::jsonDataTemplate(PRM_STRING | PRM_TYPE_NOCOOK, 1, &jsonData, &jsonDataDefault);

template <typename OP>
PRM_Name FabricDFGOP<OP>::portsChanged("__portsChanged");
template <typename OP>
PRM_Template FabricDFGOP<OP>::portsChangedTemplate(PRM_INT, 1, &portsChanged);

template <typename OP>
PRM_Name FabricDFGOP<OP>::createGraphButton("createGraph", "Create Graph and Connect");
template <typename OP>
PRM_Template FabricDFGOP<OP>::createGraphTemplate(
    PRM_CALLBACK, 1, &createGraphButton, 0, 0, 0, &FabricDFGOP<OP>::createGraphCallback);

template <typename OP>
PRM_Name FabricDFGOP<OP>::openGraphButton("openGraph", "Open Graph");
template <typename OP>
PRM_Template FabricDFGOP<OP>::openGraphButtonTemplate(
    PRM_CALLBACK, 1, &openGraphButton, 0, 0, 0, &FabricDFGOP<OP>::openGraphButtonCallback);

template <typename OP>
PRM_Name FabricDFGOP<OP>::currentFrame("currentFrame", "Current Frame");
template <typename OP>
PRM_Template FabricDFGOP<OP>::currentFrameTemplate(PRM_INT, 1, &currentFrame);

template <typename OP>
int FabricDFGOP<OP>::createGraphCallback(void* data, int index, float time, const PRM_Template* tplate)
{
    FabricDFGOP<OP>* op = reinterpret_cast<FabricDFGOP<OP>*>(data);

    std::ifstream ifs(op->getStringValue("jsonFilePath").buffer(), std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    if (buffer.str() != "")
    {
        // Store preset path
        op->setStringValue(UT_String(buffer.str()), "jsonData");
        try
        {
            op->getView().createBindingFromJSON(buffer.str());
            op->getView().setMyGraph();
            op->getView().addParametersFromInputPorts();
            op->m_graphLoaded = true;
        }
        catch (FabricCore::Exception e)
        {
            FabricCore::Exception::Throw(
                (std::string("[FabricDFGOP<OP>::createGraphCallback]: ") + e.getDesc_cstr()).c_str());
            return 0; // no need to refresh the UI if we fail
        }
    }

    return 1;
}

template <typename OP>
int FabricDFGOP<OP>::openGraphButtonCallback(void* data, int index, float time, const PRM_Template* tplate)
{
    FabricDFGOP<OP>* op = reinterpret_cast<FabricDFGOP<OP>*>(data);
    op->m_ui.show();
    return 1;
}

// If you add a new MultiParams type, you must add it to the OP too !
template <typename OP>
PRM_Template FabricDFGOP<OP>::myTemplateList[] = {
    groupTemplate,                          jsonFilePathTemplate,
    jsonDataTemplate,                       portsChangedTemplate,
    createGraphTemplate,                    openGraphButtonTemplate,
    currentFrameTemplate,

    MultiParams::Float32PortsMultiTemplate, MultiParams::SInt32PortsMultiTemplate,
    MultiParams::UInt32PortsMultiTemplate,  MultiParams::IndexPortsMultiTemplate,
    MultiParams::SizePortsMultiTemplate,    MultiParams::CountPortsMultiTemplate,
    MultiParams::StringPortsMultiTemplate,  MultiParams::FilePathPortsMultiTemplate,
    MultiParams::Vec3PortsMultiTemplate,

    PRM_Template()
};

template <typename OP>
UT_String FabricDFGOP<OP>::getStringValue(const char* name, fpreal t) const
{
    UT_String result;
    this->evalString(result, name, 0, 0, t);
    return result;
}

template <typename OP>
void FabricDFGOP<OP>::setStringValue(const UT_String& value, const char* name, fpreal t)
{
    this->setString(value, CH_STRING_LITERAL, name, 0, t);
}

template <typename OP>
void FabricDFGOP<OP>::updateGraph(const fpreal t, CopyAttributesFunc func)
{
    if (!m_graphLoaded)
    {
        // @! Set to true even if loading the graph failed.
        // This is because currently, loading the graph several times can fail
        // with error like "Error: getDesc(path): no port, graph or function named 'size'"
        m_graphLoaded = true;

        UT_String jsonData = getStringValue("jsonData");
        try
        {
            getView().createBindingFromJSON(jsonData.buffer());
            getView().setInputPortsFromOpNode(t, func); // Needed to get a "type-resolved" graph
            getView().setMyGraph();
        }
        catch (FabricCore::Exception e)
        {
            FabricCore::Exception::Throw((std::string("[FabricDFGOP<OP>::updateGraph]: ") + e.getDesc_cstr()).c_str());
        }
    }

    getView().setInputPortsFromOpNode(t, func);
}

template <typename OP>
void FabricDFGOP<OP>::executeGraph()
{
    try
    {
        getView().getBinding().execute();
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw((std::string("[FabricDFGOP<OP>::executeGraph]: ") + e.getDesc_cstr()).c_str());
    }
}

// Specializations for Houdini nodes using Fabric

template class FabricDFGOP<SOP_Node>;
template class FabricDFGOP<OBJ_Geometry>;

} // End namespace OpenSpliceHoudini
