#ifndef XMANGA_HPP
#define XMANGA_HPP
#include <QMainWindow>
#include "PageView.hpp"

namespace Ui {
  class MainWindow;
};
class QSlider;
class QLabel;

class XManga : public QMainWindow {
public:
  XManga();
  virtual ~XManga();
  bool fileOpen(const QString &fileName, int index = 0);
  void fullscreen();
  void resizeEvent(QResizeEvent *event);

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private slots:
  void imageChangedSlot(int index);

private:
  void updateBookmarkMenu();
  void updateRecentMenu();
  void connectActions();
  void addActions();
  void resetShortcuts();
  void menuPreProcess();
  void scaleModeChangedSlot(PageView::SCALE_MODE mode);
  void pageOverSlot(int index);
  QStringList getFileNames(const QString& path) const;
  void nextArchive();
  void preArchive();
  void initStatusBar(QStatusBar *status);

  Ui::MainWindow* ui_;
  PageView* pageView_;
  QSlider* pageSlider_;
  QLabel* pageLabel_;
  QList<QString> pathNameList_;
  bool fullScreen_;
  int scaleMode_;
  QString fileName_;

  QAction* actionImage_ScrollX_Plus_;
  QAction* actionImage_ScrollX_Minus_;
  QAction* actionImage_ScrollY_Plus_;
  QAction* actionImage_ScrollY_Minus_;
  QAction* actionImage_ScrollX_Plus_Or_NextPage_;
  QAction* actionImage_ScrollX_Minus_Or_PrevPage_;
};

#endif
