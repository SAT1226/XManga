#ifndef MOUSEGESTURECONFIGDIALOG_HPP
#define MOUSEGESTURECONFIGDIALOG_HPP
#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QtCore>
#include <QAction>

#include "Settings.hpp"
#include "qactionmanager/qactionmanager.h"

namespace Ui {
  class MouseGestureConfigDialog;
};

class MouseGestureConfigDialog : public QDialog {
public:
  MouseGestureConfigDialog(Settings::gestureActionManager& gestureActions,
                           const QVector<MouseGesture>& mouseGestures, QWidget *parent);
  virtual ~MouseGestureConfigDialog();
  void connectActions();
  QVector<MouseGesture> getGestures();

private:
  void setDisableEditWidgets(bool disable);
  void setGestureItem(int row, const QString& pattern, const QString& action);

  Ui::MouseGestureConfigDialog* ui_;
  Settings::gestureActionManager gestureActions_;
  QVector<MouseGesture> mouseGestures_;
};

#endif
