// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "OBJ_FabricKinematic.h"

#include <PRM/PRM_Include.h>
#include <CH/CH_LocalVariable.h>

#include <UT/UT_DMatrix3.h>

#include <ImathVec.h>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;
using std::cout;
using std::endl;
using std::boolalpha;

OP_Node* OBJ_FabricKinematic::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new OBJ_FabricKinematic(net, name, op);
}

OBJ_FabricKinematic::OBJ_FabricKinematic(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<OBJ_Geometry>(net, name, op)
{
}

OBJ_FabricKinematic::~OBJ_FabricKinematic()
{
}

// this function copies the original parameter and then adds an invisible flag
static void copyParmWithInvisible(PRM_Template& src, PRM_Template& dest)
{
    PRM_Name* new_name;

    new_name = new PRM_Name(src.getToken(), src.getLabel(), src.getExpressionFlag());
    new_name->harden();
    dest.initialize((PRM_Type)(src.getType() | PRM_TYPE_INVISIBLE),
                    src.getTypeExtended(),
                    src.exportLevel(),
                    src.getVectorSize(),
                    new_name,
                    src.getFactoryDefaults(),
                    src.getChoiceListPtr(),
                    src.getRangePtr(),
                    src.getCallback(),
                    src.getSparePtr(),
                    src.getParmGroup(),
                    (const char*)src.getHelpText(),
                    src.getConditionalBasePtr());
}

OP_TemplatePair* OBJ_FabricKinematic::buildTemplatePair(OP_TemplatePair* prevstuff)
{

    // The parm templates here are not created as a static list because
    // if that static list was built before the OBJbaseTemplate static list
    // (which it references) then that list would be corrupt. Thus we have
    // to force our static list to be created after OBJbaseTemplate.
    static PRM_Template* theTemplate = 0;

    if (!theTemplate)
    {
        PRM_Template* obj_template;
        int i;
        int size;
        UT_String parm_name;

        obj_template = OBJ_Geometry::getTemplateList(OBJ_PARMS_PLAIN);
        size = PRM_Template::countTemplates(obj_template);
        theTemplate = new PRM_Template[size + 1]; // add +1 for sentinel
        for (i = 0; i < size; i++)
        {
            theTemplate[i] = obj_template[i];
            theTemplate[i].getToken(parm_name);

            // leave only the translation parameter visible (and its containing
            // switcher)
            if (parm_name != "t" && parm_name != "stdswitcher")
                copyParmWithInvisible(obj_template[i], theTemplate[i]);
        }
    }

    // Here, we have to "inherit" template pairs from geometry and beyond. To
    // do this, we first need to instantiate our template list, then add the
    // base class templates.
    OP_TemplatePair* dfg, *geo;
    dfg = new OP_TemplatePair(myTemplateList, prevstuff);
    geo = new OP_TemplatePair(theTemplate, dfg);

    return geo;
}

int OBJ_FabricKinematic::applyInputIndependentTransform(OP_Context& context, UT_DMatrix4& mat)
{
    try
    {
        // call OBJ_Geometry::applyInputIndependentTransform() so that we don't
        // lose any information
        OBJ_Geometry::applyInputIndependentTransform(context, mat);

        fpreal now = context.getTime();
        updateGraph(now);
        executeGraph();

        FabricCore::Client client = getView().getClient();
        FabricCore::DFGBinding binding = getView().getBinding();
        FabricCore::DFGExec exec = binding.getExec();

        if (exec.getExecPortResolvedType("t") == FTL_STR("Vec3"))
        {
            float t[3];
            std::vector<FabricCore::RTVal> args(2);
            args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Float32", 3, &t);
            args[1] = FabricCore::RTVal::ConstructUInt32(client, 0 /* offset */);

            FabricCore::RTVal rtVal = binding.getArgValue("t");
            rtVal.callMethod("", "get", 2, &args[0]);
            mat.pretranslate(t[0], t[1], t[2]);
        }

        if (exec.getExecPortResolvedType("r") == FTL_STR("Vec3"))
        {
            float r[3];
            std::vector<FabricCore::RTVal> args(2);
            args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Float32", 3, &r);
            args[1] = FabricCore::RTVal::ConstructUInt32(client, 0 /* offset */);

            FabricCore::RTVal rtVal = binding.getArgValue("r");
            rtVal.callMethod("", "get", 2, &args[0]);
            mat.prerotate(r[0], r[1], r[2], UT_XformOrder());
        }

        if (exec.getExecPortResolvedType("s") == FTL_STR("Vec3"))
        {
            float s[3];
            std::vector<FabricCore::RTVal> args(2);
            args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Float32", 3, &s);
            args[1] = FabricCore::RTVal::ConstructUInt32(client, 0 /* offset */);

            FabricCore::RTVal rtVal = binding.getArgValue("s");
            rtVal.callMethod("", "get", 2, &args[0]);
            mat.prescale(s[0], s[1], s[2]);
        }

        flags().setTimeDep(true);

        // return 1 to indicate that we have modified the input matrix.
        // if we didn't modify mat, then we should return 0 instead.
        return 1;
    }
    catch (FabricCore::Exception e)
    {
        printf("FabricCore::Exception from OBJ_FabricKinematic::applyInputIndependentTransform:\n %s\n", e.getDesc_cstr());
        return 0;
    }
}
