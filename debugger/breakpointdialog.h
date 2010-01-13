#ifndef BREAKPOINTDIALOG_H
#define BREAKPOINTDIALOG_H

#include <QDialog>

#include "cregisterdata.h"
#include "cbreakpointinfo.h"
#include "cbreakpointdisplaymodel.h"
#include "cnes.h"

namespace Ui {
   class BreakpointDialog;
}

class BreakpointDialog : public QDialog {
   Q_OBJECT
public:
   BreakpointDialog(QWidget *parent = 0);
   ~BreakpointDialog();

protected:
   void changeEvent(QEvent *e);

private:
   Ui::BreakpointDialog *ui;
   CRegisterData* m_pRegister;
   CBitfieldData* m_pBitfield;
   CBreakpointDisplayModel* model;
   void DisplayBreakpoint ( int idx );

private slots:
    void on_listView_pressed(QModelIndex index);
    void on_listView_entered(QModelIndex index);
    void on_listView_doubleClicked(QModelIndex index);
    void on_listView_clicked(QModelIndex index);
    void on_listView_activated(QModelIndex index);
    void on_bitfield_currentIndexChanged(int index);
    void on_reg_currentIndexChanged(int index);
    void on_addButton_clicked();
    void on_type_currentIndexChanged(int index);
};

#endif // BREAKPOINTDIALOG_H
