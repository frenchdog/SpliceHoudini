// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "CanvasUI.h"
#include "FabricDFGView.h"
#include "FabricDFGWidget.h"

#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <RE/RE_QtWindow.h>

#include <OP/OP_Node.h>

namespace OpenSpliceHoudini
{

void CanvasUI::show()
{
    // Null ptr to get the new widget as a window
    QWidget* parent = 0;
    FabricDFGWidgetPtr dfgw(new FabricDFGWidget(parent, &m_view));
    dfgw->setOp(m_op);
    m_view.setWidget(dfgw);
    UT_WorkBuffer opFullName;
    m_op->getFullPath(opFullName);
    dfgw->setWindowTitle(opFullName.buffer());
    dfgw->show();
    dfgw->activateWindow();
}
} // End namespace OpenSpliceHoudini
