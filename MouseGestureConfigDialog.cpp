#include <QObject>
#include <QComboBox>
#include <QWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <iostream>

#include "MouseGestureConfigDialog.hpp"
#include "Settings.hpp"

#include "ui_MouseGestureConfigDialog.h"

MouseGestureConfigDialog::MouseGestureConfigDialog(Settings::gestureActionManager& gestureActions,
                                                       const QVector<MouseGesture>& mouseGestures, QWidget *parent):
  QDialog(parent), ui_(new Ui::MouseGestureConfigDialog), mouseGestures_(mouseGestures)
{
  ui_ -> setupUi(this);
  ui_ -> gestureLineEdit -> setReadOnly(true);

  QStringList list;
  list << "Gesture" << "Action" << "Action";
  ui_ -> gesturesTableWidget -> setColumnCount(3);
  ui_ -> gesturesTableWidget -> setColumnHidden(1, true);
  ui_ -> gesturesTableWidget -> setHorizontalHeaderLabels(list);
  ui_ -> gesturesTableWidget -> setSelectionBehavior(QAbstractItemView::SelectRows);
  ui_ -> gesturesTableWidget -> setSelectionMode(QAbstractItemView::SingleSelection);
  ui_ -> gesturesTableWidget -> horizontalHeader() -> setSectionResizeMode(2, QHeaderView::Stretch);
  ui_ -> gesturesTableWidget -> verticalHeader() -> hide();

  gestureActions_ = gestureActions;
  for(auto& group: gestureActions_.nameByGroups().uniqueKeys()) {
    ui_ -> groupComboBox -> addItem(group);
  }
  connectActions();
  ui_ -> groupComboBox -> activated(0);

  ui_ -> gesturesTableWidget -> setRowCount(mouseGestures_.size());
  for(int i = 0; i < mouseGestures_.size(); ++i) {
    setGestureItem(i, mouseGestures_[i].pattern, mouseGestures_[i].actionName);
  }
  ui_ -> gesturesTableWidget -> selectRow(0);

  if(ui_ -> gesturesTableWidget -> rowCount() == 0)
    setDisableEditWidgets(true);
  else
    setDisableEditWidgets(false);
}

MouseGestureConfigDialog::~MouseGestureConfigDialog()
{
}

void MouseGestureConfigDialog::setGestureItem(int row, const QString& pattern, const QString& action)
{
  auto actions = gestureActions_.actions();
  auto patternItem = new QTableWidgetItem(pattern);
  auto actionItem = new QTableWidgetItem(action);
  auto actionTextItem = new QTableWidgetItem(actions[action] -> text());

  patternItem -> setFlags(patternItem -> flags() ^ Qt::ItemIsEditable);
  actionItem -> setFlags(actionItem -> flags() ^ Qt::ItemIsEditable);
  actionTextItem -> setFlags(actionItem -> flags() ^ Qt::ItemIsEditable);

  ui_ -> gesturesTableWidget -> setItem(row, 0, patternItem);
  ui_ -> gesturesTableWidget -> setItem(row, 1, actionItem);
  ui_ -> gesturesTableWidget -> setItem(row, 2, actionTextItem);
}

void MouseGestureConfigDialog::setDisableEditWidgets(bool disable)
{
  ui_ -> upPushButton -> setDisabled(disable);
  ui_ -> groupComboBox -> setDisabled(disable);
  ui_ -> leftPushButton -> setDisabled(disable);
  ui_ -> actionComboBox -> setDisabled(disable);
  ui_ -> downPushButton -> setDisabled(disable);
  ui_ -> backPushButton -> setDisabled(disable);
  ui_ -> gestureLineEdit -> setDisabled(disable);
  ui_ -> rightPushButton -> setDisabled(disable);
  ui_ -> clearPushButton -> setDisabled(disable);
}

QVector<MouseGesture> MouseGestureConfigDialog::getGestures()
{
  return mouseGestures_;
}

void MouseGestureConfigDialog::connectActions()
{
  connect(ui_ -> gesturesTableWidget, &QTableWidget::itemSelectionChanged, [=]{
    auto row = ui_ -> gesturesTableWidget -> currentRow();

    if(row != -1) {
      auto item = ui_ -> gesturesTableWidget -> item(row, 1);
      auto actionName = item -> text();
      auto nameByGroups = gestureActions_.nameByGroups();

      ui_ -> groupComboBox -> setCurrentText(nameByGroups.keys(actionName)[0]);
      ui_ -> groupComboBox -> activated(ui_ -> groupComboBox -> currentIndex());

      int i = ui_ -> actionComboBox -> findData(actionName);
      if(i != -1)
        ui_ -> actionComboBox -> activated(i);

      item = ui_ -> gesturesTableWidget -> item(row, 0);
      ui_ -> gestureLineEdit -> setText(item -> text());
    }
  });

  connect(ui_ -> gestureLineEdit, QOverload<const QString &>::of(&QLineEdit::textChanged), [=](const QString &text) {
    auto row = ui_ -> gesturesTableWidget -> currentRow();
    if(row != -1)
      ui_ -> gesturesTableWidget -> item(row, 0) -> setText(text);
  });

  connect(ui_ -> groupComboBox, QOverload<int>::of(&QComboBox::activated), [=](int index){
    ui_ -> groupComboBox -> setCurrentIndex(index);
    ui_ -> actionComboBox -> clear();

    auto nameByGroups = gestureActions_.nameByGroups();
    auto actions = gestureActions_.actions();
    for(auto& name: nameByGroups.values(ui_ -> groupComboBox -> currentText())) {
      ui_ -> actionComboBox -> addItem(actions[name] -> text(), name);
    }

    ui_ -> actionComboBox -> activated(0);
  });

  connect(ui_ -> actionComboBox, QOverload<int>::of(&QComboBox::activated), [=](int index){
    ui_ -> actionComboBox -> setCurrentIndex(index);
    auto row = ui_ -> gesturesTableWidget -> currentRow();

    if(row != -1) {
      auto item = ui_ -> gesturesTableWidget -> item(row, 2);
      item -> setText(ui_ -> actionComboBox -> currentText());

      item = ui_ -> gesturesTableWidget -> item(row, 1);
      item -> setText(ui_ -> actionComboBox -> currentData().toString());
    }
  });

  connect(ui_ -> addPushButton, &QPushButton::clicked, [=]{
    auto row = ui_ -> gesturesTableWidget -> rowCount();
    ui_ -> gesturesTableWidget -> setRowCount(row + 1);
    setGestureItem(row, "", ui_ -> actionComboBox -> currentData().toString());

    ui_ -> gesturesTableWidget -> selectRow(row);
    ui_ -> groupComboBox -> activated(0);
    ui_ -> actionComboBox -> activated(0);
    ui_ -> gestureLineEdit -> clear();
    setDisableEditWidgets(false);
  });

  connect(ui_ -> removePushButton, &QPushButton::clicked, [=]{
    auto row = ui_ -> gesturesTableWidget -> currentRow();
    ui_ -> gesturesTableWidget -> removeRow(row);

    if(ui_ -> gesturesTableWidget -> rowCount() == 0)
      setDisableEditWidgets(true);
  });

  connect(ui_ -> upPushButton, &QPushButton::clicked, [=]{
    ui_ -> gestureLineEdit -> setText(ui_ -> gestureLineEdit -> text() + "U");
  });

  connect(ui_ -> downPushButton, &QPushButton::clicked, [=]{
    ui_ -> gestureLineEdit -> setText(ui_ -> gestureLineEdit -> text() + "D");
  });

  connect(ui_ -> rightPushButton, &QPushButton::clicked, [=]{
    ui_ -> gestureLineEdit -> setText(ui_ -> gestureLineEdit -> text() + "R");
  });

  connect(ui_ -> leftPushButton, &QPushButton::clicked, [=]{
    ui_ -> gestureLineEdit -> setText(ui_ -> gestureLineEdit -> text() + "L");
  });

  connect(ui_ -> backPushButton, &QPushButton::clicked, [=]{
    auto text = ui_ -> gestureLineEdit -> text();
    text.chop(1);
    ui_ -> gestureLineEdit -> setText(text);
  });

  connect(ui_ -> clearPushButton, &QPushButton::clicked, [=]{
    ui_ -> gestureLineEdit -> clear();
  });

  connect(this, &QDialog::finished, [=]{
    mouseGestures_.clear();
    for(int i = 0; i < ui_ -> gesturesTableWidget -> rowCount(); ++i) {
      MouseGesture mouseGesture;
      mouseGesture.pattern = ui_ -> gesturesTableWidget -> item(i, 0) -> text();
      mouseGesture.actionName = ui_ -> gesturesTableWidget -> item(i, 1) -> text();
      mouseGestures_.push_back(mouseGesture);
    }
  });
}
