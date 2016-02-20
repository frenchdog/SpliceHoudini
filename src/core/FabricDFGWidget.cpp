// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "FabricDFGWidget.h"
#include "FabricDFGView.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include <OP/OP_Node.h>

namespace OpenSpliceHoudini
{

std::map<FabricDFGView*, FabricDFGWidget*> FabricDFGWidget::s_instances;

FabricDFGWidget::FabricDFGWidget(QWidget* parent, FabricDFGView* view)
    : DFG::DFGCombinedWidget(parent)
    , m_view(view)
{
    if (m_view)
    {
        FabricCore::DFGBinding binding = m_view->getBinding();
        FabricCore::DFGExec exec = binding.getExec();
        init(m_view->getClient(), m_view->getManager(), m_view->getHost(), binding, FTL::StrRef(), exec, m_view->getStack(), false);
    }
}

FabricDFGWidget::~FabricDFGWidget()
{
    for (std::map<FabricDFGView*, FabricDFGWidget*>::iterator it = s_instances.begin(); it != s_instances.end(); it++)
    {
        if (it->second == this)
        {
            s_instances.erase(it);
            break;
        }
    }
}

FabricDFGWidget* FabricDFGWidget::getWidgetforDFGView(FabricDFGView* view, bool createNewIfNoneFound)
{
    std::map<FabricDFGView*, FabricDFGWidget*>::iterator it = s_instances.find(view);
    if (it == s_instances.end())
    {
        // don't create new widget?
        if (!createNewIfNoneFound)
            return NULL;

        // create new widget.
        QMainWindow* mainWindow = NULL;
        FabricDFGWidget* newWidget = new FabricDFGWidget(mainWindow, view);
        s_instances.insert(std::pair<FabricDFGView*, FabricDFGWidget*>(view, newWidget));
        Qt::WindowFlags flags = (*newWidget).windowFlags();
        (*newWidget).setWindowFlags(flags | Qt::WindowStaysOnTopHint);

        return newWidget;
    }
    return it->second;
}

void FabricDFGWidget::setOp(OP_Node* op)
{
    m_op = op;
}

void FabricDFGWidget::onRecompilation()
{
    // m_op->setString(UT_String(m_view->getJSON().c_str()), CH_STRING_LITERAL, "jsonData", 0, 0);

    // int val = m_op->evalInt("__portsChanged", 0, 0);
    // m_op->setInt("__portsChanged", 0, 0, (val + 1) % 2);
}

void FabricDFGWidget::onPortRenamed(QString path, QString newName)
{
}

} // End namespace OpenSpliceHoudini
