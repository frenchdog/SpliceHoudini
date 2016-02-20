// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __FabricDFGOP_H__
#define __FabricDFGOP_H__

#include "MultiParams.h"
#include "FabricDFGView.h"
#include "CanvasUI.h"

#include <OP/OP_Operator.h>
#include <OP/OP_Network.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_SpareData.h>

#include <fstream>
#include <sstream>

using namespace FabricServices;

namespace OpenSpliceHoudini
{

class CanvasUI;

/// Use this class as a base for any custom operator (SOP/ROP/GEO/etc...)
/// integrating Fabric DFG.
/// It will provide the functionality to create the graph and connect it to the
/// Houdini world.
template <typename OP>
class FabricDFGOP : public OP
{
public:
    FabricDFGOP(OP_Network* net, const char* name, OP_Operator* op);

    // static PRM_Name groupName;
    static PRM_Template groupTemplate;

    static PRM_Name jsonFilePath;
    static PRM_Default jsonFilePathDefault;
    static PRM_Template jsonFilePathTemplate;

    static PRM_Name jsonData;
    static PRM_Default jsonDataDefault;
    static PRM_Template jsonDataTemplate;

    static PRM_Name portsChanged;
    static PRM_Template portsChangedTemplate;

    static PRM_Name createGraphButton;
    static PRM_Template createGraphTemplate;
    static int createGraphCallback(void* data, int index, float time, const PRM_Template* tplate);

    static PRM_Name openGraphButton;
    static PRM_Template openGraphButtonTemplate;
    static int openGraphButtonCallback(void* data, int index, float time, const PRM_Template* tplate);

    /// Hidden parameter set with the "$F" expression
    static PRM_Name currentFrame;
    static PRM_Template currentFrameTemplate;

    static PRM_Template myTemplateList[];

    UT_String getStringValue(const char* name, fpreal t = 0) const;

protected:
    void setStringValue(const UT_String& value, const char* name, fpreal t = 0);

    FabricDFGView& getView()
    {
        return m_view;
    }

    /// Setup the Canvas graph and set its ports values from Houdini data
    void updateGraph(const fpreal t, CopyAttributesFunc func = 0);
    /// Evaluate the Canvas graph
    void executeGraph();

    bool m_graphLoaded;

private:
    FabricDFGView m_view;
    CanvasUI m_ui;
};

} // End namespace OpenSpliceHoudini
#endif // __FabricDFGOP_H__
