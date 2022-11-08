#include "XManga.hpp"

#include "Archive.hpp"
#include "Settings.hpp"
#include "ui_MainWindow.h"
#include "BookmarkDialog.hpp"

XManga::XManga() : ui_(new Ui::MainWindow),
                   fullScreen_(false), scaleMode_(1)
{
  qApp -> installEventFilter(this);
  ui_ -> setupUi(this);

  ui_ -> statusbar -> setSizeGripEnabled(false);
  initStatusBar(ui_ -> statusbar);

  pageView_ = ui_ -> graphicsView;
  connect(pageView_, &PageView::imageChanged, this, &XManga::imageChangedSlot);
  connect(pageView_, &PageView::pageOver, this, &XManga::pageOverSlot);
  connect(pageView_, &PageView::scaleModeChanged, this, &XManga::scaleModeChangedSlot);
  connect(pageView_, &PageView::dndFileName, this, [=](const QString &fileName) {
                                                     fileOpen(fileName);
                                                   });
  pageView_ -> setMargin(0);

  setAcceptDrops(true);
  connectActions();

  Settings::Instance().keyActions.addDefaultKey("actionOpen", QKeySequence("Ctrl+O"));
  Settings::Instance().keyActions.addDefaultKey("actionClearRecent", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionQuit", QKeySequence("Q"));
  Settings::Instance().keyActions.addDefaultKey("actionFullScreen", QKeySequence("F"));
  Settings::Instance().keyActions.addDefaultKey("actionDouble_Page", QKeySequence("Space"));
  Settings::Instance().keyActions.addDefaultKey("actionShow_only_one_page_where_appropriate", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionBest_Fit", QKeySequence("B"));
  Settings::Instance().keyActions.addDefaultKey("actionFit_Width", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionFit_Height", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionManual_Zoom", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("MagnifyingLens", QKeySequence("Ctrl+L"));
  Settings::Instance().keyActions.addDefaultKey("actionZoom_In", QKeySequence("+"));
  Settings::Instance().keyActions.addDefaultKey("actionZoom_out", QKeySequence("-"));
  Settings::Instance().keyActions.addDefaultKey("actionNormal_Size", QKeySequence("="));
  Settings::Instance().keyActions.addDefaultKey("actionKey_Config", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionNearest_Neighbor", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionBiLinear", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionLanczos3", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionNext", QKeySequence("PgDown, N"));
  Settings::Instance().keyActions.addDefaultKey("actionPrev", QKeySequence("PgUp, P"));
  Settings::Instance().keyActions.addDefaultKey("actionFirstPage", QKeySequence("Home"));
  Settings::Instance().keyActions.addDefaultKey("actionLastPage", QKeySequence("End"));
  Settings::Instance().keyActions.addDefaultKey("actionNext_One_Page_Only", QKeySequence("Shift+Right, Shift+L"));
  Settings::Instance().keyActions.addDefaultKey("actionPrev_One_Page_Only", QKeySequence("Shift+Left, Shift+H"));
  Settings::Instance().keyActions.addDefaultKey("actionNext_Volume", QKeySequence("Ctrl+PgDown, Ctrl+N"));
  Settings::Instance().keyActions.addDefaultKey("actionPrev_Volume", QKeySequence("Ctrl+PgUp, Ctrl+P"));
  Settings::Instance().keyActions.addDefaultKey("actionImage_ScrollX_Plus", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionImage_ScrollX_Minus", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionImage_ScrollY_Plus", QKeySequence("Down, J"));
  Settings::Instance().keyActions.addDefaultKey("actionImage_ScrollY_Minus", QKeySequence("Up, K"));
  Settings::Instance().keyActions.addDefaultKey("actionImage_ScrollX_Minus_Or_PrevPage", QKeySequence("Left, H"));
  Settings::Instance().keyActions.addDefaultKey("actionImage_ScrollX_Plus_Or_NextPage", QKeySequence("Right, L"));
  Settings::Instance().keyActions.addDefaultKey("actionEdit_Bookmarks", QKeySequence(""));
  Settings::Instance().keyActions.addDefaultKey("actionAdd_Bookmark", QKeySequence(""));

  Settings::Instance().keyActions.registAction("actionQuit", ui_ -> actionQuit, "File");
  Settings::Instance().keyActions.registAction("actionClearRecent", ui_ -> actionClear_Recent, "File");
  Settings::Instance().keyActions.registAction("actionOpen", ui_ -> actionOpen, "File");
  Settings::Instance().keyActions.registAction("actionKey_Config", ui_ -> actionKey_Config, "View");
  Settings::Instance().keyActions.registAction("actionNormal_Size", ui_ -> actionNormal_Size, "View");
  Settings::Instance().keyActions.registAction("actionZoom_out", ui_ -> actionZoom_out, "View");
  Settings::Instance().keyActions.registAction("actionZoom_In", ui_ -> actionZoom_In, "View");
  Settings::Instance().keyActions.registAction("MagnifyingLens", ui_ -> actionMagnifyingLens, "View");
  Settings::Instance().keyActions.registAction("actionManual_Zoom", ui_ -> actionManual_Zoom, "View");
  Settings::Instance().keyActions.registAction("actionFit_Height", ui_ -> actionFit_Height, "View");
  Settings::Instance().keyActions.registAction("actionFit_Width", ui_ -> actionFit_Width, "View");
  Settings::Instance().keyActions.registAction("actionBest_Fit", ui_ -> actionBest_Fit, "View");
  Settings::Instance().keyActions.registAction("actionShow_only_one_page_where_appropriate", ui_ -> actionShow_only_one_page_where_appropriate, "View");
  Settings::Instance().keyActions.registAction("actionDouble_Page", ui_ -> actionDouble_Page, "View");
  Settings::Instance().keyActions.registAction("actionFullScreen", ui_ -> actionFullScreen, "View");
  Settings::Instance().keyActions.registAction("actionLanczos3", ui_ -> actionLanczos3, "Scale");
  Settings::Instance().keyActions.registAction("actionBiLinear", ui_ -> actionBiLinear, "Scale");
  Settings::Instance().keyActions.registAction("actionNearest_Neighbor", ui_ -> actionNearest_Neighbor, "Scale");
  Settings::Instance().keyActions.registAction("actionPrev_Volume", ui_ -> actionPrev_Volume, "Navigation");
  Settings::Instance().keyActions.registAction("actionNext_Volume", ui_ -> actionNext_Volume, "Navigation");
  Settings::Instance().keyActions.registAction("actionPrev_One_Page_Only", ui_ -> actionPrev_One_Page_Only, "Navigation");
  Settings::Instance().keyActions.registAction("actionNext_One_Page_Only", ui_ -> actionNext_One_Page_Only, "Navigation");
  Settings::Instance().keyActions.registAction("actionLastPage", ui_ -> actionLastPage, "Navigation");
  Settings::Instance().keyActions.registAction("actionFirstPage", ui_ -> actionFirstPage, "Navigation");
  Settings::Instance().keyActions.registAction("actionPrev", ui_ -> actionPrev, "Navigation");
  Settings::Instance().keyActions.registAction("actionNext", ui_ -> actionNext, "Navigation");
  Settings::Instance().keyActions.registAction("actionImage_ScrollX_Plus_Or_NextPage", actionImage_ScrollX_Plus_Or_NextPage_, "Image");
  Settings::Instance().keyActions.registAction("actionImage_ScrollX_Minus_Or_PrevPage", actionImage_ScrollX_Minus_Or_PrevPage_, "Image");
  Settings::Instance().keyActions.registAction("actionImage_ScrollY_Minus", actionImage_ScrollY_Minus_, "Image");
  Settings::Instance().keyActions.registAction("actionImage_ScrollY_Plus", actionImage_ScrollY_Plus_, "Image");
  Settings::Instance().keyActions.registAction("actionImage_ScrollX_Minus", actionImage_ScrollX_Minus_, "Image");
  Settings::Instance().keyActions.registAction("actionImage_ScrollX_Plus", actionImage_ScrollX_Plus_, "Image");
  Settings::Instance().keyActions.registAction("actionEdit_Bookmarks", ui_ -> actionEdit_Bookmarks, "Bookmark");
  Settings::Instance().keyActions.registAction("actionAdd_Bookmark", ui_ -> actionAdd_Bookmark, "Bookmark");

  Settings::Instance().Load();
  pageView_ -> setSpread(Settings::Instance().spread);
  ui_ -> actionDouble_Page -> setChecked(Settings::Instance().spread);

  if(!Settings::Instance().spread)
    ui_ -> actionShow_only_one_page_where_appropriate -> setDisabled(true);

  ui_ -> actionShow_only_one_page_where_appropriate -> setChecked(
      Settings::Instance().autoOnePage);
  pageView_ -> setAutoOnePage(Settings::Instance().autoOnePage);

  switch(static_cast<PageView::SCALE_ALGORITHMS>(
      Settings::Instance().scaleAlgorithms)) {
  case PageView::NEAREST_NEIGHBOR:
    ui_ -> actionNearest_Neighbor -> activate(QAction::Trigger);
    break;
  case PageView::BILINEAR:
    ui_ -> actionBiLinear -> activate(QAction::Trigger);
    break;
  case PageView::LANCZOS3:
    ui_ -> actionLanczos3 -> activate(QAction::Trigger);
    break;
  };

  int w, h;
  if (Settings::Instance().width == -1)
    w = width();
  else
    w = Settings::Instance().width;
  if (Settings::Instance().height == -1)
    h = height();
  else
    h = Settings::Instance().height;

  resize(w, h);
  updateRecentMenu();
  updateBookmarkMenu();
  addActions();
  resetShortcuts();
}

XManga::~XManga()
{
  std::cerr << "XManga::~Destructor!!" << std::endl;
  Settings::Instance().spread = pageView_ -> getSpread();
  Settings::Instance().scaleAlgorithms = pageView_ -> getScaleAlgorithms();

  auto size = normalGeometry();
  Settings::Instance().width = size.width();
  Settings::Instance().height = size.height();
  Settings::Instance().autoOnePage = pageView_ -> getAutoOnePage();
  Settings::Instance().Save();
}

bool XManga::fileOpen(const QString &fileName, int index)
{
  if(pageView_ -> open(fileName, index)) {
    fileName_ = fileName;
    pathNameList_.clear();

    pathNameList_ = pageView_ -> getPathNameList();
    pageSlider_ -> setMaximum(pageView_ -> getPathNameList().size() - 1);
    pageSlider_ -> setValue(0);
    pageLabel_ -> setText("0/" + QString::number(pageView_ -> getPathNameList().size() - 1));

    Settings::Instance().AddRecent(fileName_);
    updateRecentMenu();

    QFileInfo archiveFileName(fileName_);
    setWindowTitle("XManga: " + archiveFileName.fileName());
  }
  else {
    qDebug() << "FileOpen Error: " << fileName;
    QMessageBox::critical(this, "", "FileOpen Error: " + fileName);

    fileName_ = "";
    setWindowTitle("XManga:");
    pageSlider_ -> setValue(0);
    pageLabel_ -> setText("0/0");

    return false;
  }
  return true;
}

void XManga::fullscreen() {
  if(!fullScreen_) {
    fullScreen_ = true;
    ui_ -> menubar -> hide();
    ui_ -> statusbar -> hide();
    showFullScreen();
    pageView_ -> setMargin(0);
    pageView_ -> resetImage();
  }
  else {
    fullScreen_ = false;
    ui_ -> menubar -> show();
    ui_ -> statusbar -> show();
    showNormal();
    pageView_ -> setMargin(0);
    pageView_ -> resetImage();
  }

  ui_ -> actionFullScreen -> setChecked(fullScreen_);
}

void XManga::resizeEvent(QResizeEvent *event)
{
  if(event -> spontaneous()) {
    pageView_ -> resetImage();
  }
}

bool XManga::eventFilter(QObject *obj, QEvent *event)
{
  if(fullScreen_) {
    if(event -> type() == QEvent::MouseMove) {
      QMouseEvent *e = static_cast<QMouseEvent *>(event);
      auto rect = this -> rect();

      // status Bar
      auto statusBar = ui_ -> statusbar;
      if(!statusBar -> isVisible() && rect.height() - 5 <= e -> windowPos().y()) {
        statusBar -> setVisible(true);
        pageView_ -> setMargin(statusBar -> height());
      }

      if(statusBar -> isVisible()) {
        if(qApp -> mouseButtons() == Qt::NoButton) {
          int margin = statusBar -> height();
          auto scrollBar = pageView_ -> horizontalScrollBar();
          if (scrollBar -> isVisible())
            margin += scrollBar -> height();

          if (rect.height() - margin > e -> windowPos().y()) {
            statusBar -> setVisible(false);
            pageView_ -> setMargin(0);
          }
        }
      }

      // Menu Bar
      auto menuBar = ui_ -> menubar;
      if(!statusBar -> isVisible() && !menuBar -> isVisible() && 5 >= e -> windowPos().y()) {
        if(QApplication::mouseButtons() == Qt::NoButton) {
          menuBar -> setVisible(true);
          pageView_ -> setOffsetY(pageView_ -> getOffset().y() -
                                (menuBar -> height()));
        }
      }
      if(menuBar -> isVisible()) {
        if(qApp -> mouseButtons() == Qt::NoButton) {
          auto widget = qApp -> widgetAt(QCursor::pos());
          if(!(dynamic_cast<QMenu *>(widget) != 0 ||
                dynamic_cast<QMenuBar *>(widget) != 0)) {
            menuBar -> setVisible(false);
            ui_ -> menuFile -> hide();
            ui_ -> menuRecent -> hide();
            ui_ -> menuScale -> hide();
            ui_ -> menuView -> hide();
            ui_ -> menuZoom -> hide();
            ui_ -> menuNavigation -> hide();
            ui_ -> menuBookmark -> hide();
            pageView_ -> resetOffsetY();
          }
        }
      }
      return false;
    }
  }

  return false;
}

void XManga::imageChangedSlot(int index)
{
  QFileInfo archiveFileName(fileName_);
  QFileInfo imageFileName(pathNameList_[index]);

  setWindowTitle("XManga: " + archiveFileName.fileName() + " : " + imageFileName.fileName());
  pageSlider_ -> setValue(index);
  pageLabel_ -> setText(QString::number(index) + "/" +
                      QString::number(pageView_ -> getPathNameList().size() - 1));
}

void XManga::updateBookmarkMenu()
{
  auto func = [=](QString name) {
    auto n = name.indexOf(":");
    auto index = name.left(n).toInt();
    auto fileName = name.mid(n + 1, -1);

    menuPreProcess();
    fileOpen(fileName, index);
  };

  ui_ -> menuBookmark -> clear();
  ui_ -> menuBookmark -> addAction(ui_ -> actionAdd_Bookmark);
  ui_ -> menuBookmark -> addAction(ui_ -> actionEdit_Bookmarks);
  ui_ -> menuBookmark -> addSeparator();

  for(auto name : Settings::Instance().bookmark) {
    auto action = ui_ -> menuBookmark -> addAction(name);
    connect(action, &QAction::triggered, std::bind(func, name));
  }
}

void XManga::updateRecentMenu()
{
  ui_ -> menuRecent -> clear();
  auto func = [=](QString name) {
    menuPreProcess();
    fileOpen(name);
  };

  auto it = Settings::Instance().recent.constEnd();
  while(it != Settings::Instance().recent.constBegin()) {
    QString name = *(--it);
    auto action = ui_ -> menuRecent -> addAction(name);
    connect(action, &QAction::triggered, std::bind(func, name));
  }

  ui_ -> menuRecent -> addSeparator();
  ui_ -> menuRecent -> addAction(ui_ -> actionClear_Recent);
}

void XManga::connectActions()
{
  // File
  connect(ui_ -> actionOpen, &QAction::triggered, [=] {
    menuPreProcess();
    QFileInfo f(fileName_);

    auto fileName = QFileDialog::getOpenFileName(
        this, "", f.absolutePath(), "Archive (*.zip *.rar *.cbz *.cbr)");

    if(!fileName.isEmpty()) {
      fileOpen(fileName);
    }
  });
  connect(ui_ -> actionClear_Recent, &QAction::triggered, [=] {
    Settings::Instance().recent.clear();
    updateRecentMenu();
  });
  connect(ui_ -> actionQuit, &QAction::triggered, [=] { QApplication::quit(); });

  // View
  connect(ui_ -> actionFullScreen, &QAction::triggered, [=] {
    menuPreProcess();
    fullscreen();
  });
  connect(ui_ -> actionDouble_Page, &QAction::triggered, [=](bool checked) {
    menuPreProcess();
    pageView_ -> setSpread(checked);

    if(checked)
      ui_ -> actionShow_only_one_page_where_appropriate -> setDisabled(false);
    else
      ui_ -> actionShow_only_one_page_where_appropriate -> setDisabled(true);
  });

  connect(ui_ -> actionShow_only_one_page_where_appropriate, &QAction::triggered,
          [=](bool checked) {
            menuPreProcess();
            pageView_ -> setAutoOnePage(checked);
            pageView_ -> refreshImage();
          });

  auto actions = new QActionGroup(this);
  actions -> addAction(ui_ -> actionBest_Fit);
  actions -> addAction(ui_ -> actionFit_Width);
  actions -> addAction(ui_ -> actionFit_Height);
  actions -> addAction(ui_ -> actionManual_Zoom);
  ui_ -> actionBest_Fit -> setChecked(true);
  connect(actions, &QActionGroup::triggered, [=](QAction *action) {
    menuPreProcess();

    if(action == ui_ -> actionBest_Fit) {
      pageView_ -> setScaleMode(PageView::BEST_FIT);
    }
    if(action == ui_ -> actionFit_Width) {
      pageView_ -> setScaleMode(PageView::FIT_WIDTH);
    }
    if(action == ui_ -> actionFit_Height) {
      pageView_ -> setScaleMode(PageView::FIT_HEIGHT);
    }
    if(action == ui_ -> actionManual_Zoom) {
      pageView_ -> setScaleMode(PageView::MANUAL_ZOOM);
      pageView_ -> setScaleRatio(1.0);
    }
    pageView_ -> resetImage();
  });

  connect(ui_ -> actionMagnifyingLens, &QAction::triggered, [=](bool checked) {
    menuPreProcess();
    pageView_ -> setLupe(checked);
  });

  connect(ui_ -> actionZoom_In, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScaleRatio(pageView_ -> getScaleRatio() + 0.05);
    pageView_ -> setScaleMode(PageView::MANUAL_ZOOM);
    pageView_ -> resetImage();
  });
  connect(ui_ -> actionZoom_out, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScaleRatio(pageView_ -> getScaleRatio() - 0.05);
    pageView_ -> setScaleMode(PageView::MANUAL_ZOOM);
    pageView_ -> resetImage();
  });
  connect(ui_ -> actionNormal_Size, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScaleRatio(1.0);
    pageView_ -> setScaleMode(PageView::MANUAL_ZOOM);
    pageView_ -> resetImage();
  });
  connect(ui_ -> actionKey_Config, &QAction::triggered, [=] {
    menuPreProcess();
    KeyConfigDialog dialog(Settings::Instance().keyActions, this);
    if(dialog.exec() == QDialog::Accepted) {
      Settings::Instance().keyActions = dialog.keyActions();
      resetShortcuts();
    }
  });

  // Scale
  actions = new QActionGroup(this);
  actions -> addAction(ui_ -> actionNearest_Neighbor);
  actions -> addAction(ui_ -> actionBiLinear);
  actions -> addAction(ui_ -> actionLanczos3);
  ui_ -> actionBiLinear -> setChecked(true);
  connect(actions, &QActionGroup::triggered, [=](QAction *action) {
    menuPreProcess();

    if(action == ui_ -> actionNearest_Neighbor) {
      pageView_ -> setScaleAlgorithms(PageView::NEAREST_NEIGHBOR);
      scaleMode_ = 0;
    }
    if(action == ui_ -> actionBiLinear) {
      pageView_ -> setScaleAlgorithms(PageView::BILINEAR);
      scaleMode_ = 1;
    }
    if(action == ui_ -> actionLanczos3) {
      pageView_ -> setScaleAlgorithms(PageView::LANCZOS3);
      scaleMode_ = 2;
    }

    pageView_ -> resetImage();
  });

  // Navigation
  connect(ui_ -> actionNext, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> nextPage();
  });
  connect(ui_ -> actionPrev, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> prePage();
  });
  connect(ui_ -> actionFirstPage, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScrolledPos(PageView::START);
    pageView_ -> setPage(0);
  });
  connect(ui_ -> actionLastPage, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScrolledPos(PageView::START);
    pageView_ -> setPage(pageView_ -> getPathNameList().size() - 1);
  });
  connect(ui_ -> actionNext_One_Page_Only, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScrolledPos(PageView::START);
    pageView_ -> setPage(pageView_ -> getIndex() + 1);
  });
  connect(ui_ -> actionPrev_One_Page_Only, &QAction::triggered, [=] {
    menuPreProcess();
    pageView_ -> setScrolledPos(PageView::START);
    pageView_ -> setPage(pageView_ -> getIndex() - 1);
  });
  connect(ui_ -> actionNext_Volume, &QAction::triggered, [=] {
    menuPreProcess();
    nextArchive();
  });
  connect(ui_ -> actionPrev_Volume, &QAction::triggered, [=] {
    menuPreProcess();
    preArchive();
  });

  // Bookmark
  connect(ui_ -> actionAdd_Bookmark, &QAction::triggered, [=] {
    menuPreProcess();
    if(fileName_ != "") {
      Settings::Instance().bookmark.push_back(
          QString::number(pageSlider_ -> value()) + ":" + fileName_);
      updateBookmarkMenu();
    }
  });
  connect(ui_ -> actionEdit_Bookmarks, &QAction::triggered, [=] {
    menuPreProcess();
    BookmarkDialog dialog(Settings::Instance().bookmark, this);
    if(dialog.exec() == QDialog::Accepted) {
      Settings::Instance().bookmark = dialog.getBookmarks();
      updateBookmarkMenu();
    }
  });

  // Image
  actionImage_ScrollX_Plus_ = new QAction("ScrollX_Plus", this);
  actionImage_ScrollX_Minus_ = new QAction("ScrollX_Minus", this);
  actionImage_ScrollY_Plus_ = new QAction("ScrollY_Plus", this);
  actionImage_ScrollY_Minus_ = new QAction("ScrollY_Minus", this);
  actionImage_ScrollX_Plus_Or_NextPage_ = new QAction("ScrollX_Plus_Or_NextPage", this);
  actionImage_ScrollX_Minus_Or_PrevPage_ = new QAction("ScrollX_Minus_Or_PrevPage", this);

  connect(actionImage_ScrollX_Plus_, &QAction::triggered,
          [=] { pageView_ -> scrollX(40); });
  connect(actionImage_ScrollX_Minus_, &QAction::triggered,
          [=] { pageView_ -> scrollX(-40); });
  connect(actionImage_ScrollY_Plus_, &QAction::triggered,
          [=] { pageView_ -> scrollY(40); });
  connect(actionImage_ScrollY_Minus_, &QAction::triggered,
          [=] { pageView_ -> scrollY(-40); });
  connect(actionImage_ScrollX_Plus_Or_NextPage_, &QAction::triggered, [=] {
    if(!pageView_ -> horizontalScrollBar() -> isVisible()) {
      pageView_ -> nextPage();
    }
    else {
      pageView_ -> scrollX(40);
    }
  });
  connect(actionImage_ScrollX_Minus_Or_PrevPage_, &QAction::triggered, [=] {
    if(!pageView_ -> horizontalScrollBar() -> isVisible()) {
      pageView_ -> prePage();
    }
    else {
      pageView_ -> scrollX(-40);
    }
  });
}

void XManga::addActions()
{
  auto actions = Settings::Instance().keyActions.actions();
  for(auto name : actions.keys()) {
    this -> addAction(actions[name]);
  }
}

void XManga::resetShortcuts()
{
  auto actions = Settings::Instance().keyActions.actions();
  auto seqMap = Settings::Instance().keyActions.keyMaps();
  for(auto name : actions.keys()) {
    auto act = actions[name];
    auto seq = seqMap[name];

    QList<QKeySequence> seqlist;
    for(int i = 0; i < seq.count(); i++)
      seqlist.append(QKeySequence(seq[i]));

    act -> setShortcuts(seqlist);
  }
}

void XManga::menuPreProcess()
{
  if(fullScreen_) {
    ui_ -> menubar -> hide();
    layout() -> activate();
    pageView_ -> resetOffsetY();
  }
}

void XManga::scaleModeChangedSlot(PageView::SCALE_MODE mode)
{
  switch(mode) {
  case PageView::BEST_FIT:
    ui_ -> actionBest_Fit -> setChecked(true);
    break;
  case PageView::FIT_WIDTH:
    ui_ -> actionFit_Width -> setChecked(true);
    break;
  case PageView::FIT_HEIGHT:
    ui_ -> actionFit_Height -> setChecked(true);
    break;
  case PageView::MANUAL_ZOOM:
    ui_ -> actionManual_Zoom -> setChecked(true);
    break;
  };
}

void XManga::pageOverSlot(int index)
{
  if(index < 0) {
    preArchive();
  }
  if(index > 0) {
    nextArchive();
  }
}

QStringList XManga::getFileNames(const QString& path) const
{
  QDir dir = QDir(path);
  auto fileList = dir.entryList();

  QCollator sorter;
  sorter.setNumericMode(true);
  sorter.setCaseSensitivity(Qt::CaseInsensitive);
  std::sort(fileList.begin(), fileList.end(), sorter);

  return fileList;
}

void XManga::nextArchive()
{
  if(pageView_ -> isLoading()) return;

  QFileInfo f(fileName_);
  auto fileList = getFileNames(f.absolutePath());
  Archive arcTest;

  for(int i = 0; i < fileList.size(); ++i) {
    if(fileList[i] == f.fileName()) {
      for(int j = i + 1; j < fileList.size(); ++j) {
        if(j < fileList.size()) {
          QFileInfo file(f.absolutePath(), fileList[j]);
          if(!file.isFile()) continue;

          if(arcTest.test(file.absoluteFilePath())) {
            if(fileOpen(file.absoluteFilePath())) {
              return;
            }
          }
        }
      }
    }
  }
  //fileOpen(f.absoluteFilePath(), lastPage);
}

void XManga::preArchive()
{
  if (pageView_ -> isLoading()) return;

  QFileInfo f(fileName_);
  auto fileList = getFileNames(f.absolutePath());
  Archive arcTest;

  for(int i = 0; i < fileList.size(); ++i) {
    if(fileList[i] == f.fileName()) {
      for(int j = i - 1; j > 0; --j) {
        if(j >= 0) {
          QFileInfo file(f.absolutePath(), fileList[j]);
          if(!file.isFile()) continue;

          if(arcTest.test(file.absoluteFilePath())) {
            if(fileOpen(file.absoluteFilePath())) {
              return;
            }
          }
        }
      }
    }
  }
  //fileOpen(f.absoluteFilePath());
}

void XManga::initStatusBar(QStatusBar *status)
{
  auto widget = new QWidget();
  widget -> setContentsMargins(0, 0, 0, 0);
  auto layout = new QHBoxLayout();
  layout -> setContentsMargins(0, 0, 0, 0);
  layout -> setSpacing(0);
  widget -> setLayout(layout);

  auto icon = QApplication::style() -> standardIcon(QStyle::SP_ArrowLeft);
  auto button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked, [&] { preArchive(); });
  layout -> addWidget(button);

  icon = QApplication::style() -> standardIcon(QStyle::SP_MediaSeekBackward);
  button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked, [&] {
    pageView_ -> setScrolledPos(PageView::START);
    pageView_ -> setPage(pageView_ -> getIndex() - 1);
  });
  layout -> addWidget(button);

  icon = QApplication::style() -> standardIcon(QStyle::SP_MediaSeekForward);
  button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked, [&] {
    pageView_ -> setScrolledPos(PageView::START);
    pageView_ -> setPage(pageView_ -> getIndex() + 1);
  });
  layout -> addWidget(button);

  icon = QApplication::style() -> standardIcon(QStyle::SP_ArrowRight);
  button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked, [&] { nextArchive(); });
  layout -> addWidget(button);
  status -> addWidget(widget);

  icon = QApplication::style() -> standardIcon(QStyle::SP_DialogOpenButton);
  button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked, ui_ -> actionOpen, &QAction::triggered);
  status -> addWidget(button);

  icon = QApplication::style() -> standardIcon(QStyle::SP_MediaSkipBackward);
  button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked, [&] { pageView_ -> setPage(0); });
  status -> addWidget(button);

  pageSlider_ = new QSlider(Qt::Horizontal);
  pageSlider_ -> setRange(0, 255);
  pageSlider_ -> setTickPosition(QSlider::TicksAbove);
  pageSlider_ -> setTickInterval(10);
  pageSlider_ -> setMaximum(0);
  pageSlider_ -> setTracking(false);
  connect(pageSlider_, &QSlider::sliderMoved, [&](int value) {
    auto pos = pageSlider_ -> mapToGlobal(QPoint(pageSlider_ -> width() / 2, 5));
    pos.setX(QCursor::pos().x() + 5);
    QToolTip::showText(pos, QString::number(value), this);
  });
  connect(pageSlider_, &QSlider::valueChanged,
          [&](int value) { pageView_ -> setPage(value); });
  status -> addWidget(pageSlider_, 1);

  icon = QApplication::style() -> standardIcon(QStyle::SP_MediaSkipForward);
  button = new QPushButton(icon, "");
  connect(button, &QPushButton::clicked,
          [&] { pageView_ -> setPage(pathNameList_.size() - 1); });
  status -> addWidget(button);

  pageLabel_ = new QLabel("0/0");
  pageLabel_ -> setMargin(5);
  status -> addWidget(pageLabel_);
}
