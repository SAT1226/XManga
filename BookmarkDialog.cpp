#include <Qt>
#include <QDebug>
#include <QStringListModel>
#include "BookmarkDialog.hpp"
#include "ui_BookmarkDialog.h"

class UnEditableStringListModel: public QStringListModel {
public:
  virtual Qt::ItemFlags flags(const QModelIndex &index) const {
    auto defaultFlags = QStringListModel::flags(index);
    if (index.isValid()){
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
    return defaultFlags;
  }
private:
};

BookmarkDialog::BookmarkDialog(const QStringList& bookmarks,
                               QWidget *parent) : QDialog(parent),
                                                  ui_(new Ui::BookmarkDialog)
{
  ui_ -> setupUi(this);

  model_ = new UnEditableStringListModel();
  model_ -> setStringList(bookmarks);
  ui_ -> bookmarkView -> setModel(model_);

  ui_ -> bookmarkView -> setDragEnabled(true);
  ui_ -> bookmarkView -> setAcceptDrops(true);
  ui_ -> bookmarkView -> setDropIndicatorShown(true);
  ui_ -> bookmarkView -> setDragDropMode(QAbstractItemView::InternalMove);
  ui_ -> bookmarkView -> setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui_ -> bookmarkView -> setDefaultDropAction(Qt::MoveAction);

  connectActions();
}

BookmarkDialog::~BookmarkDialog()
{
}

QStringList BookmarkDialog::getBookmarks() const
{
  return model_ -> stringList();
}

void BookmarkDialog::connectActions() {
  connect(ui_->removeButton, &QPushButton::clicked, [=] {
    auto selectedIndexes =
        ui_->bookmarkView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selectedIndexes) {
      model_->removeRow(index.row());
    }
  });

  connect(ui_->downButton, &QPushButton::clicked, [=] {
    auto selectedIndexes =
        ui_->bookmarkView->selectionModel()->selectedIndexes();
    auto lst = model_->stringList();

    int row = 0;
    for (const QModelIndex &index : selectedIndexes) {
      if (index.row() + 1 < model_->rowCount()) {
        lst.swap(index.row(), index.row() + 1);
        row = index.row() + 1;
      } else {
        row = model_->rowCount() - 1;
      }
    }
    model_->setStringList(lst);

    auto index = model_->index(row, 0);
    ui_->bookmarkView->setCurrentIndex(index);
  });

  connect(ui_->upButton, &QPushButton::clicked, [=] {
    auto selectedIndexes =
        ui_->bookmarkView->selectionModel()->selectedIndexes();
    auto lst = model_->stringList();

    int row = 0;
    for (const QModelIndex &index : selectedIndexes) {
      if (index.row() - 1 >= 0) {
        lst.swap(index.row(), index.row() - 1);
        row = index.row() - 1;
      } else {
        row = 0;
      }
    }
    model_->setStringList(lst);

    auto index = model_->index(row, 0);
    ui_->bookmarkView->setCurrentIndex(index);
  });
}
