#include <QApplication>
#include <QGraphicsView>
#include <QMutex>
#include <QQueue>
#include <QtConcurrent/QtConcurrentRun>
#include <QMainWindow>
#include <PageView.hpp>
#include <QPushButton>
#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QDebug>
#include <QMimeData>
#include <cmath>

#include "./blitz/blitz.h"

PageView::PageView(QWidget *parent) :
  QGraphicsView(parent), index_(-1), thread_(true), setImage_(true),
  spreadMode_(false), lupeMode_(false), margin_(0), pagePixmapItem_(0),
  scaleAlgorithms_(SCALE_ALGORITHMS::BILINEAR), scaleMode_(SCALE_MODE::BEST_FIT),
  scaleRatio_(1.0), scrollCnt_(0), onePageOnly_(false), pindex_(-1), sindex1_(0), sindex2_(0)
{
  QGraphicsScene *scene = new QGraphicsScene;
  this -> setScene(scene);
  setBackgroundBrush(QBrush("0", Qt::SolidPattern));
  this -> setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  connect(this, &PageView::privateImageChanged, this, &PageView::imageChangedSlot);

  setDragMode(ScrollHandDrag);

  future_ = QtConcurrent::run(this, &PageView::run);
  setAcceptDrops( true );
}

PageView::~PageView()
{
  thread_ = false;
  future_.waitForFinished();
}

void PageView::dragEnterEvent(QDragEnterEvent *event)
{
  auto mime = event -> mimeData();

  if (mime->hasUrls()) event->accept();
  else event->ignore();
}

void PageView::dropEvent(QDropEvent *event)
{
  auto mime = event->mimeData();

  if(mime->hasUrls()){
    auto filename = event -> mimeData() -> urls()[0].path(QUrl::FullyDecoded);

    emit dndFileName(filename);
  }
}

void PageView::dragMoveEvent(QDragMoveEvent *event)
{
}

void PageView::dragLeaveEvent(QDragLeaveEvent *event)
{
}

bool PageView::refreshImage()
{
  if(spreadMode_) {
    int p1 = index_;
    int p2 = index_ + 1;

    if(index_ == 0) p1 = p2 = index_;
    if(p2 >= pathNameList_.size()) p2 = pathNameList_.size() - 1;
    if(p1 >= pathNameList_.size()) p1 = pathNameList_.size() - 1;

    if(imageCache_.checkImage(p1) && imageCache_.checkImage(p2)) {
      auto img1 = imageCache_.getImage(p1);
      auto img2 = imageCache_.getImage(p2);
      auto rect1 = img1.rect();
      auto rect2 = img2.rect();

      if(autoOnePage_) {
        if (rect2.height() < rect2.width() ||
            rect1.height() < rect1.width()) {
          onePageOnly_ = true;
          p1 = pindex_;
          p2 = -1;
          // index_ = oldIndex = p1;
          index_ = p1;
        }
      }
      emit privateImageChanged(p1, p2);
      return true;
    }
  }
  else {
    if(imageCache_.checkImage(index_)) {
      emit privateImageChanged(index_, -1);
      return true;
    }
  }

  return false;
}

bool PageView::isLoading() const
{
  return !setImage_;
}
void PageView::setScaleRatio(double ratio)
{
  scaleRatio_ = ratio;
}

double PageView::getScaleRatio() const
{
  return scaleRatio_;
}

bool PageView::getAutoOnePage() const
{
  return autoOnePage_;
}

void PageView::setAutoOnePage(bool autoOnePage)
{
  autoOnePage_ = autoOnePage;
}

void PageView::setMargin(int m)
{
  margin_ = m;
}

void PageView::setOffset(int x, int y)
{
  pagePixmapItem_ -> setOffset(x, y);
}

void PageView::setOffsetX(int x)
{
  auto offset = pagePixmapItem_ -> offset();
  pagePixmapItem_ -> setOffset(x, offset.y());
}

QPointF PageView::getOffset() const
{
  if(pagePixmapItem_ != 0) {
    return pagePixmapItem_ -> offset();
  }
  return QPointF(0.0, 0.0);
}

void PageView::setOffsetY(int y)
{
  auto offset = pagePixmapItem_ -> offset();
  pagePixmapItem_ -> setOffset(offset.x(), y);
}

void PageView::resetOffsetY()
{
  auto rect = this -> scene() -> sceneRect();
  int y = (rect.height() - pagePixmapItem_ -> pixmap().rect().height()) / 2;

  setOffsetY(y);
}

void PageView::setLupe(bool lupe)
{
  lupeMode_ = lupe;
  setMouseTracking(lupe);

  if(lupe) {
    lupe_ = this -> scene() -> addPixmap(QPixmap::fromImage(QImage()));

    auto point = mapToScene(mapFromGlobal(QCursor::pos()));
    moveLupeItem(point.x(), point.y());

  }
  else {
    this -> scene() -> removeItem(lupe_);
    lupe_ = 0;
  }
}

bool PageView::getLupe() const
{
  return lupeMode_;
}

int PageView::getIndex() const
{
  return index_;
}

bool PageView::getSpread() const
{
  return spreadMode_;
}

void PageView::showEvent(QShowEvent *event)
{
  this -> resetImage();
}

void PageView::setSpread(bool spread)
{
  spreadMode_ = spread;
  if(spread) onePageOnly_ = false;

  thread_ = false;
  future_.waitForFinished();

  thread_ = true;
  setImage_ = false;
  future_ = QtConcurrent::run(this, &PageView::run);
}

QList<QString> PageView::getPathNameList() const
{
  return pathNameList_;
}

void PageView::setScaleAlgorithms(SCALE_ALGORITHMS mode)
{
  scaleAlgorithms_ = mode;
}

PageView::SCALE_ALGORITHMS PageView::getScaleAlgorithms() const
{
  return scaleAlgorithms_;
}

void PageView::setScaleMode(SCALE_MODE mode)
{
  scaleMode_ = mode;
  emit scaleModeChanged(mode);
}

PageView::SCALE_MODE PageView::getScaleMode() const
{
  return scaleMode_;
}

void PageView::imageChangedSlot(int p1, int p2)
{
  sindex1_ = p1, sindex2_ = p2;
  if(spreadMode_) {
    auto img1 = imageCache_.getImage(p1);
    QImage img2;

    if(p2 != -1) img2 = imageCache_.getImage(p2);
    auto rect1 = img1.rect();
    auto rect2 = img2.rect();

    auto w = rect1.width() + rect2.width();
    auto h = rect1.height() > rect2.height() ? rect1.height() : rect2.height();

    QImage destImg = QImage(w, h, QImage::Format_RGB32);
    destImg.fill(0x000000);

    QPainter painter(&destImg);
    painter.drawImage(0, 0, img2);
    if(!(p1 == p2)) {
      painter.drawImage(img2.width(), 0, img1);
    }
    painter.end();

    image_ = destImg;
    setImage(destImg);
  }
  else {
    auto img = imageCache_.getImage(p1);
    image_ = img;
    setImage(img);
  }

  if(scrolledPos_ == START)
    this -> centerOn(0, 0);
  else
    this -> centerOn(0, sceneRect().height());

  if(lupeMode_) {
    auto point = mapToScene(mapFromGlobal(QCursor::pos()));
    moveLupeItem(point.x(), point.y());
  }

  setImage_ = true;
  QApplication::restoreOverrideCursor();

  emit imageChanged(index_);
}

void PageView::run()
{
  int oldIndex = -1;
  QQueue<int> pageQueue;
  bool emited = false;
  auto pushQueue = [=](QQueue<int>& queue, int x) {
                    if(x < 0) return;
                    if(x == oldIndex) return;
                    if(x >= pathNameList_.size()) return;

                    queue.push_back(x);
                  };

  while(thread_) {
    if(index_ == -1) {
      QThread::msleep(1);
      continue;
    }
    if(oldIndex != index_) {
      oldIndex = index_;
      emited = false;
      pageQueue.clear();
      pageQueue.push_back(oldIndex);

      int r = spreadMode_ ? 6 : 3;

      for(int i = 1; i <= r; ++i) {
        pushQueue(pageQueue, oldIndex + i);
        pushQueue(pageQueue, oldIndex - i);
      }
      imageCache_.removeImage(oldIndex, r);
    }
    else {
      if(!pageQueue.empty()) {
        int page = pageQueue.front();
        pageQueue.pop_front();

        if(!imageCache_.checkImage(page)) {
          auto img = QImage::fromData(archive_.getData(pathNameList_[page]));

//          if(img.size().width() > 800)
//            img = img.scaledToWidth(800, Qt::SmoothTransformation);

          imageCache_.setImage(page, img);
        }
      }
    }
    QThread::msleep(1);

    if(!setImage_ && !emited) emited = refreshImage();
  }
}

QImage PageView::scaleImage(int w, int h, QImage& img)
{
  if(scaleAlgorithms_ == NEAREST_NEIGHBOR) {
    if(h == -1)
      return img.scaledToWidth(w, Qt::FastTransformation);
    else if(w == -1)
      return img.scaledToHeight(h, Qt::FastTransformation);
    else
      return img.scaled(w, h, Qt::KeepAspectRatio, Qt::FastTransformation);
  }
  else if (scaleAlgorithms_ == BILINEAR) {
    if(h == -1)
      return img.scaledToWidth(w, Qt::SmoothTransformation);
    else if(w == -1)
      return img.scaledToHeight(h, Qt::SmoothTransformation);
    else
      return img.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }
  else {
    auto rect = img.rect();
    double w_ratio = static_cast<double>(w) / static_cast<double>(rect.width());
    double h_ratio = static_cast<double>(h) / static_cast<double>(rect.height());
    double ratio = 0.0;

    if(h == -1) h_ratio = w_ratio;
    if(w == -1) w_ratio = h_ratio;

    if(w_ratio > h_ratio) ratio = h_ratio;
    else ratio = w_ratio;

    auto size = QSize(floor(rect.width() * ratio), floor(rect.height() * ratio));

    return Blitz::smoothScaleFilter(img, size, 1.0, Blitz::LanczosFilter);
  }
}

void PageView::resetImage()
{
  auto rect = this -> rect();
  auto srect = this -> verticalScrollBar() -> sizeHint();
  rect.setWidth(this -> contentsRect().width());
  rect.setHeight(this -> contentsRect().height() + margin_);

  QImage img;

  if(scaleMode_ == BEST_FIT) {
    img = scaleImage(rect.width(), rect.height(), image_);
    rect = img.rect();
  }
  else if(scaleMode_ == FIT_WIDTH) {
    img = scaleImage(rect.width(), -1, image_);
    rect = img.rect();
  }
  else if(scaleMode_ == FIT_HEIGHT) {
    img = scaleImage(-1, rect.height(), image_);
    rect = img.rect();
  }
  else {
    img = scaleImage(round(image_.rect().width() * scaleRatio_),
                     round(image_.rect().height() * scaleRatio_), image_);
    rect = img.rect();
  }

  if(img.rect().width() != 0) {
    scaleRatio_ = static_cast<double>(img.rect().width()) / static_cast<double>(image_.width());
  }

  if(img.rect().width() > this -> contentsRect().width()) rect.setWidth(img.rect().width());
  else {
    if(this -> contentsRect().height() + margin_ < img.rect().height()) {
      rect.setWidth(this -> contentsRect().width() - srect.width());
    }
    else {
      rect.setWidth(this -> contentsRect().width());
      rect.setHeight(this -> contentsRect().height() + margin_);
    }
  }

  this -> scene() -> clear();
  this -> scene() -> setSceneRect(rect);
  pagePixmapItem_ = this -> scene() -> addPixmap(QPixmap::fromImage(img));

  int x = (rect.width() - img.rect().width()) / 2;
  int y = (rect.height() - img.rect().height()) / 2;

  pagePixmapItem_ -> setOffset(x, y);

  if(lupeMode_) {
    auto point = mapToScene(mapFromGlobal(QCursor::pos()));
    lupe_ = this -> scene() -> addPixmap(QPixmap::fromImage(QImage()));

    moveLupeItem(point.x(), point.y());
  }
}

void PageView::setImage(const QImage& image)
{
  image_ = image;
  this -> resetImage();
}

bool PageView::open(const QString& fileName, int index = 0)
{
  thread_ = false;
  future_.waitForFinished();

  pathNameList_.clear();
  imageCache_.clear();

  index_ = -1;
  thread_ = true;
  setImage_ = true;
  future_ = QtConcurrent::run(this, &PageView::run);

  if(!archive_.Open(fileName)) {
    thread_ = false;
    future_.waitForFinished();

    pathNameList_ = archive_.getPathNameList();
    image_ = QImage();
    resetImage();

    return false;
  }

  image_ = QImage();
  resetImage();
  pathNameList_ = archive_.getPathNameList();
  scrolledPos_ = START;
  setPage(index);

  return true;
}

void PageView::setPage(int index)
{
  if(!setImage_) return;
  if(pathNameList_.size() == 0) return;
  if(index >= pathNameList_.size()) {
    index = pathNameList_.size() - 1;
    emit pageOver(pathNameList_.size());
    return;
  }
  if(index < 0) {
    index = 0;
    emit pageOver(-1);
    return;
  }

  if(index != index_) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    index_ = pindex_ = index;
    onePageOnly_ = false;
    setImage_ = false;
    scrollCnt_ = 0;
  }
}

void PageView::setScrolledPos(SCROLLED_POS pos)
{
  scrolledPos_ = pos;
}

void PageView::nextPage(bool wheel)
{
  if(!setImage_) return;

  scrolledPos_ = START;

  if(pathNameList_.empty()) emit pageOver(1);
  if(spreadMode_){
    if(index_ == 0)
      setPage(index_ + 1);
    else if (onePageOnly_) {
      setPage(index_ + 1);
    }
    else {
      if(index_ + 2 < pathNameList_.size()) {
        setPage(index_ + 2);
      }
      else {
        if(!(sindex1_ == pathNameList_.size() - 1 ||
             sindex2_ == pathNameList_.size() - 1)) {
          setPage(pathNameList_.size() - 1);
        }
        else {
          if(wheel) {
            if(scrollCnt_ == 2) {
              scrollCnt_ = 0;
              emit pageOver(pathNameList_.size());
            }
          }
          else {
            scrollCnt_ = 0;
            emit pageOver(pathNameList_.size());
          }
        }
      }
    }
  }
  else setPage(index_ + 1);
}

void PageView::prePage(bool wheel)
{
  if(!setImage_) return;

  scrolledPos_ = END;
  if(pathNameList_.empty()) emit pageOver(-1);

  if(spreadMode_){
    if(index_ - 2 >= 0) {
      if(index_ - 2 == 0) {
        setPage(index_ - 1);
      }
      else {
        setPage(index_ - 2);
        pindex_ = index_ + 1;
      }
    }
    else {
      if(index_ != 0)
        setPage(0);
      else {
        if(wheel) {
          if(scrollCnt_ == 2) {
            scrollCnt_ = 0;
            emit pageOver(-1);
          }
        }
        else {
          scrollCnt_ = 0;
          emit pageOver(-1);
        }
      }
    }
  }
  else setPage(index_ - 1);
}

void PageView::moveLupeItem(int x, int y)
{
  if(lupeMode_) {
    auto rect1 = pagePixmapItem_ -> pixmap().rect();
    auto rect = image_.rect();

    double ratio = (double)rect.width() / (double)rect1.width();
    auto offsetPoint = pagePixmapItem_ -> offset();

    int x1 = (x - 50 - offsetPoint.x()) * ratio;
    int y1 = (y - 50 - offsetPoint.y()) * ratio;
    int x2 = (x + 50 - offsetPoint.x()) * ratio;
    int y2 = (y + 50 - offsetPoint.y()) * ratio;

    lupe_ -> setPos(x - 100, y - 100);
    QImage cropped = image_.copy(x1, y1, x2 - x1, y2 - y1);

    auto img = cropped.scaled(200, 200,
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation);

    QPainter painter(&img);
    painter.setPen(QColor(100,100,100));
    painter.drawRect(0, 0, 199, 199);
    painter.end();

    lupe_ -> setPixmap(QPixmap::fromImage(img));
  }
}

void PageView::mouseMoveEvent(QMouseEvent *event)
{
  QGraphicsView::mouseMoveEvent(event);

  if(lupeMode_) {
    auto point = mapToScene(event -> pos());
    moveLupeItem(point.x(), point.y());
  }
}

void PageView::scrollX(int x)
{
  auto bar = horizontalScrollBar();
  bar -> setValue(bar -> value() + x);

  if(lupeMode_) {
    auto point = mapToScene(mapFromGlobal(QCursor::pos()));
    moveLupeItem(point.x(), point.y());
  }
}

void PageView::scrollY(int y)
{
  auto bar = verticalScrollBar();
  bar -> setValue(bar -> value() + y);

  if(lupeMode_) {
    auto point = mapToScene(mapFromGlobal(QCursor::pos()));
    moveLupeItem(point.x(), point.y());
  }
}

void PageView::wheelEvent(QWheelEvent * event)
{
  auto degress = event -> angleDelta() / 8;

  if(event -> modifiers() == Qt::ControlModifier) {
    if(degress.y() > 0) scaleRatio_ += 0.05;
    if(degress.y() < 0) scaleRatio_ -= 0.05;

    setScaleMode(PageView::MANUAL_ZOOM);
    if(setImage_) resetImage();
    return;
  }
  if(event->modifiers() == Qt::ShiftModifier) {
    if(setImage_) {
      if(degress.y() > 0) {
        setScrolledPos(PageView::START);
        setPage(getIndex() - 1);
      }
      if(degress.y() < 0) {
        setScrolledPos(PageView::START);
        setPage(getIndex() + 1);
      }
      return;
    }
  }

  ++scrollCnt_;
  if(degress.y() > 0) {
    if(setImage_) prePage(true);
  }
  if(degress.y() < 0) {
    if(setImage_) nextPage(true);
  }
}

void PageView::keyPressEvent(QKeyEvent *event)
{
  QWidget::keyPressEvent(event);

  return;
}
