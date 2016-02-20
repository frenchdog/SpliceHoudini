// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef _FABRICDFGWIDGET_H_
#define _FABRICDFGWIDGET_H_

#define FEC_SHARED
#define FECS_SHARED

#include "FabricDFGView.fwd.h"

#include <QtGui/QWidget>
#include <QtGui/QSplitter>

// #include <DFGWrapper/DFGWrapper.h>
#include <ASTWrapper/KLASTManager.h>
#include <Commands/CommandStack.h>
#include <map>

#include <FabricUI/DFG/DFGUI.h>
#include <FabricUI/DFG/DFGLogWidget.h>
#include <FabricUI/DFG/DFGCombinedWidget.h>
#include <FabricUI/DFG/DFGValueEditor.h>

using namespace FabricServices;
using namespace FabricUI;

class OP_Node;

namespace OpenSpliceHoudini
{

class FabricDFGWidget : public DFG::DFGCombinedWidget
{
    Q_OBJECT

public:
    FabricDFGWidget(QWidget* parent, FabricDFGView* view);
    ~FabricDFGWidget();

    void setOp(OP_Node* op);
    static FabricDFGWidget* getWidgetforDFGView(FabricDFGView* view, bool createNewIfNoneFound = true);

public slots:
    virtual void onRecompilation();
    virtual void onPortRenamed(QString path, QString newName);

private:
    static std::map<FabricDFGView*, FabricDFGWidget*> s_instances;
    OP_Node* m_op;
    FabricDFGView* m_view;
};
} // End namespace OpenSpliceHoudini

#endif
