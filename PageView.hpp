#ifndef PAGEVIEW_HPP
#define PAGEVIEW_HPP

#include <QResizeEvent>
#include <QGraphicsView>
#include <QFuture>
#include <QMutex>
#include <iostream>

#include "Archive.hpp"

class ImageCache {
public:
  ImageCache() {}

  bool checkImage(int index) {
    QMutexLocker locker(&mutex_);

    auto iter = imageMap_.find(index);
    if(iter != imageMap_.end()) return true;
    else return false;
  }

  void clear() {
    QMutexLocker locker(&mutex_);
    imageMap_.clear();
  }

  void setImage(int index, const QImage& image) {
    QMutexLocker locker(&mutex_);
    imageMap_[index] = image;
  }

  void removeImage(int index, int rage) {
    QMutexLocker locker(&mutex_);

    auto list = imageMap_.keys();
    for(auto i: list) {
      if(!((i >= (index - rage)) && (i <= (index + rage)))) {
        std::cerr << "remove :" << i << std::endl;
        imageMap_.remove(i);
      }
    }
  }

  void removeImage(int index) {
    QMutexLocker locker(&mutex_);
    imageMap_.remove(index);
  }

  QImage getImage(int index) {
    QMutexLocker locker(&mutex_);

    auto iter = imageMap_.find(index);
    if(iter != imageMap_.end()) {
      std::cerr << "Found: " << index << std::endl;
      return imageMap_.value(index);
    }
    else {
      QImage img;
      std::cerr << "Not Found..: " << index << std::endl;
      return img;
    }
  }

private:
  QMap<int, QImage> imageMap_;
  QMutex mutex_;
};

class PageView : public QGraphicsView {
Q_OBJECT
public:
  PageView(QWidget *parent = Q_NULLPTR);
  virtual ~PageView();

  void resetImage();
  bool refreshImage();

  void setImage(const QImage& image);
  void wheelEvent(QWheelEvent * event);
  void setPage(int index);
  bool open(const QString& arcName, int index, const QString& fileName);
  void keyPressEvent(QKeyEvent *event);
  QList<QString> getPathNameList() const;
  void nextPage(bool wheel = false);
  void prePage(bool wheel = false);
  void setSpread(bool spread);
  bool getSpread() const ;
  void mouseMoveEvent(QMouseEvent *event);
  void showEvent(QShowEvent *event);
  void setMargin(int m);
  QPointF getOffset() const;
  void setOffset(int x, int y);
  void setOffsetX(int x);
  void setOffsetY(int y);
  void resetOffsetY();
  void scrollX(int x);
  void scrollY(int y);
  int getIndex() const;
  void setLupe(bool lupe);
  bool getLupe() const;
  bool isLoading() const;
  void setScaleRatio(double ratio);
  double getScaleRatio() const;

  enum SCALE_ALGORITHMS {
     NEAREST_NEIGHBOR,
     BILINEAR,
     LANCZOS3,
  };
  void setScaleAlgorithms(SCALE_ALGORITHMS mode);
  SCALE_ALGORITHMS getScaleAlgorithms() const;

  enum SCALE_MODE {
    BEST_FIT,
    FIT_WIDTH,
    FIT_HEIGHT,
    MANUAL_ZOOM,
  };
  void setScaleMode(SCALE_MODE mode);
  SCALE_MODE getScaleMode() const;

  enum SCROLLED_POS {
      START,
      END
  };
  void setScrolledPos(SCROLLED_POS pos);
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dragLeaveEvent(QDragLeaveEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent *event);

  bool getAutoOnePage() const;
  void setAutoOnePage(bool autoOnePage);

private:
  void moveLupeItem(int x, int y);
  void run();
  QImage scaleImage(int w, int h, QImage& img);

  QImage image_;
  Archive archive_;
  ImageCache imageCache_;
  QList<QString> pathNameList_;

  QAtomicInt index_;
  QFuture<void> future_;
  QAtomicInteger<bool> thread_;
  QAtomicInteger<bool> setImage_;
  QAtomicInteger<bool> spreadMode_;
  bool lupeMode_;
  int margin_;
  QGraphicsPixmapItem *lupe_;
  QGraphicsPixmapItem *pagePixmapItem_;
  SCALE_ALGORITHMS scaleAlgorithms_;
  SCALE_MODE scaleMode_;
  double scaleRatio_;
  int scrollCnt_;
  SCROLLED_POS scrolledPos_;
  bool onePageOnly_;
  int pindex_;
  bool autoOnePage_;
  int sindex1_, sindex2_;
  bool drag_;

signals:
  void privateImageChanged(int p1, int p2);
  void imageChanged(int index);
  void pageOver(int index);
  void scaleModeChanged(SCALE_MODE mode);
  void dndFileName(const QString& filename);

private slots:
  void imageChangedSlot(int p1, int p2);
};

#endif
