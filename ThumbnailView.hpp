#ifndef THUMBNAILVIEW_HPP
#define THUMBNAILVIEW_HPP
#include <QListView>
#include <QtConcurrent/QtConcurrentRun>
#include <cstddef>
#include <iostream>

#include "Archive.hpp"

class ThumbnailModel : public QAbstractListModel {
Q_OBJECT
public:
  ThumbnailModel(QList<QString> pathNameList, QWidget *parent = Q_NULLPTR) : QAbstractListModel(parent), pathNameList_(pathNameList) {
  }

  int rowCount(const QModelIndex &parent = QModelIndex()) const {
    (void)parent;
    return pathNameList_.size();
  }

  QVariant data(const QModelIndex &index , int role = Qt::DisplayRole) const;
  void addImage(int index, QImage img);

  virtual ~ThumbnailModel() {
    std::cerr << "ThumbnailModel Destructor!!" << std::endl;
  }

private:
  QList<QString> pathNameList_;
  QMap<int, QImage> imgMap_;
};

class ThumbnailView : public QListView {
Q_OBJECT
public:
  ThumbnailView(QWidget *parent = Q_NULLPTR);
  virtual ~ThumbnailView();

  bool open(const QString &arcName);
  virtual bool event(QEvent* event);
  void setOffsetY(int y);
  void clear();

private:
  void run(Archive* archive, QList<QString> pathNameList);

  QAtomicInteger<bool> thread_;
  QAtomicInteger<bool> show_;
  QFuture<void> future_;
  Archive archive_;
  QList<QString> pathNameList_;

private slots:
  void addImage(int index, QImage img);

signals:
  void loadedImg(int index, QImage img);
};

#endif
