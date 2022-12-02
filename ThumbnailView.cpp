#include "ThumbnailView.hpp"
#include "qnamespace.h"
#include <QScrollBar>
#include <QKeyEvent>
#include <QPainter>

QVariant ThumbnailModel::data(const QModelIndex &index , int role) const
{
  auto row = index.row();

  if(role == Qt::DisplayRole) {
    return QVariant();
  }

  if(role == Qt::DecorationRole) {
    if(imgMap_.find(row) == imgMap_.end()) {
      QPixmap pixmap(192, 192);
      pixmap.fill(Qt::blue);
      return pixmap;
    }
    else {
      QPixmap pixmap(192, 192);
      pixmap.fill("#272822");

      QPainter paint(&pixmap);
      auto image = imgMap_[row];
      paint.drawImage((192 - image.width()) / 2, (192 - image.height()) / 2, image);

      return pixmap;
    }
  }

  return QVariant();
}

void ThumbnailModel::addImage(int index, QImage img)
{
  imgMap_[index] = img;
  emit layoutChanged();
}

ThumbnailView::ThumbnailView(QWidget *parent) : QListView(parent), thread_(true), show_(false)
{
  this -> setResizeMode(QListView::Adjust);
  this -> setViewMode(IconMode);
  this -> setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  this -> verticalScrollBar() -> setSingleStep(64);
  this -> setStyleSheet("QListView{background-color: #272822}");

  connect(this, &ThumbnailView::loadedImg, this, &ThumbnailView::addImage);
  future_ = QtConcurrent::run([this](){ this -> run(&archive_, QList<QString>()); });
}

ThumbnailView::~ThumbnailView()
{
  thread_ = false;
  future_.waitForFinished();
}

void ThumbnailView::setOffsetY(int y)
{
  setViewportMargins(0, y, 0, 0);
}

bool ThumbnailView::event(QEvent* event)
{
  if(event -> type() == QEvent::Show) {
    show_ = true;
  }
  if(event -> type() == QEvent::Hide) {
    show_ = false;
  }

  if(event -> type() == QEvent::ShortcutOverride) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if(keyEvent -> modifiers() == Qt::NoModifier && (keyEvent -> key() == Qt::Key_Left   || keyEvent -> key() == Qt::Key_PageDown ||
                                                     keyEvent -> key() == Qt::Key_PageUp || keyEvent -> key() == Qt::Key_Right ||
                                                     keyEvent -> key() == Qt::Key_Up     || keyEvent -> key() == Qt::Key_Down ||
                                                     keyEvent -> key() == Qt::Key_Home   || keyEvent -> key() == Qt::Key_End ||
                                                     keyEvent -> key() == Qt::Key_J      || keyEvent -> key() == Qt::Key_K ||
                                                     keyEvent -> key() == Qt::Key_H      || keyEvent -> key() == Qt::Key_L ||
                                                     keyEvent -> key() == Qt::Key_Space)) {

      if(keyEvent -> key() == Qt::Key_J) {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QListView::event(&ev);
      }
      if(keyEvent -> key() == Qt::Key_K) {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QListView::event(&ev);
      }
      if(keyEvent -> key() == Qt::Key_H) {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
        QListView::event(&ev);
      }
      if(keyEvent -> key() == Qt::Key_L) {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
        QListView::event(&ev);
      }
      if(keyEvent -> key() == Qt::Key_Space) {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QListView::event(&ev);
      }

      event -> accept();
      return true;
    }
  }

  return QListView::event(event);
}

void ThumbnailView::clear()
{
  thread_ = false;
  future_.waitForFinished();
  pathNameList_.clear();

  auto *selectionModel = this -> selectionModel();
  ThumbnailModel* model = new ThumbnailModel(pathNameList_);
  if(this -> model() != nullptr) this -> model() -> deleteLater();
  this -> setModel(model);
  delete selectionModel;
}

bool ThumbnailView::open(const QString &arcName)
{
  thread_ = false;
  future_.waitForFinished();

  if(!archive_.open(arcName)) {
    thread_ = false;
    future_.waitForFinished();
    pathNameList_.clear();
    return false;
  }

  auto *selectionModel = this -> selectionModel();
  pathNameList_ = archive_.getPathNameList();
  ThumbnailModel* model = new ThumbnailModel(pathNameList_);
  if(this -> model() != nullptr) this -> model() -> deleteLater();
  this -> setModel(model);
  delete selectionModel;

  thread_ = true;
  future_ = QtConcurrent::run([this](){ this -> run(&archive_, pathNameList_); });

  return true;
}

void ThumbnailView::addImage(int index, QImage img)
{
  auto model = dynamic_cast<ThumbnailModel*>(this -> model());
  model -> addImage(index, img);
}

void ThumbnailView::run(Archive* archive, QList<QString> pathNameList)
{
  int i = 0;
  while(thread_) {
    QThread::msleep(1);

    if(!show_) continue;
    if(pathNameList.size() == 0) continue;
    if(pathNameList.size() > i) {

      auto img = QImage::fromData(archive -> getData(pathNameList.at(i)));

      if(img.width() > img.height())
        img = img.scaledToWidth(192, Qt::SmoothTransformation);
      else
        img = img.scaledToHeight(192, Qt::SmoothTransformation);

      emit loadedImg(i++, img);
    }
  }
}
