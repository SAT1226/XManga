#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QVector>

#include "Singleton.hpp"
#include "qactionmanager/keyconfigdialog.h"

struct MouseGesture {
  QString pattern;
  QString actionName;
};

class Settings : public Singleton<Settings> {
  friend Singleton<Settings>;
public:
  QActionManager<QKeySequence, QKeySequence, QAction*> keyActions;

  typedef QActionManager<QKeySequence, QString, QAction*> gestureActionManager;
  gestureActionManager gestureActions;
  QVector<MouseGesture> mouseGestures;

  bool spread;
  int scaleAlgorithms;
  int width, height;
  QStringList recent, bookmark;
  bool autoOnePage;

  Settings() {
    settings_ = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              "SAT1226", "xmanga");
    settings_ -> setIniCodec(QTextCodec::codecForName("UTF-8"));
  }

  void Load() {
    settings_ -> beginGroup("KeyConfig");
    foreach(const QString& action, settings_ -> childKeys()) {
        QString str = settings_ -> value(action, "").toString();
        keyActions.updateKey(action,  QKeySequence(str), true);
    }
    settings_ -> endGroup();
    spread          = settings_ -> value("Spread", false).toBool();
    scaleAlgorithms = settings_ -> value("ScaleAlgorithms", 1).toInt();
    recent          = settings_ -> value("Recent", QStringList()).toStringList();
    bookmark        = settings_ -> value("Bookmark", QStringList()).toStringList();
    width           = settings_ -> value("Width", -1).toInt();
    height          = settings_ -> value("Height", -1).toInt();
    autoOnePage     = settings_ -> value("AutoOnePage", false).toBool();

    int size = settings_ -> beginReadArray("MouseGestures");
    for(int i = 0; i < size; ++i) {
      settings_ -> setArrayIndex(i);

      MouseGesture gesture;
      gesture.pattern = settings_ -> value("Pattern", "").toString();
      gesture.actionName = settings_ -> value("Action", "").toString();
      mouseGestures.push_back(gesture);
    }
    settings_ -> endArray();
  }

  void Save() {
    settings_ -> beginGroup("KeyConfig");
    for(auto action: keyActions.keyMaps().keys()) {
        settings_ -> setValue(action, keyActions.keyMaps()[action].toString());
    }
    settings_ -> endGroup();

    settings_ -> beginGroup("MouseGesture");
    settings_ -> remove("");
    settings_ -> endGroup();

    settings_ -> beginWriteArray("MouseGestures");
    for(int i = 0; i < mouseGestures.size(); ++i) {
      settings_ -> setArrayIndex(i);
      settings_ -> setValue("Pattern", mouseGestures[i].pattern);
      settings_ -> setValue("Action", mouseGestures[i].actionName);
    }
    settings_ -> endArray();

    settings_ -> setValue("Spread", spread);
    settings_ -> setValue("ScaleAlgorithms", scaleAlgorithms);
    settings_ -> setValue("Recent", recent);
    settings_ -> setValue("Bookmark", bookmark);
    settings_ -> setValue("Width", width);
    settings_ -> setValue("Height", height);
    settings_ -> setValue("AutoOnePage", autoOnePage);
  }

  void AddRecent(const QString& fileName){
    for(auto name : recent)
      if(name == fileName) return;

    if(recent.size() == 10) {
      recent.removeFirst();
    }
    recent.push_back(fileName);
  }

  ~Settings() {
    qDebug() << settings_ -> fileName();
    settings_ -> sync();
  }

private:
  QSettings* settings_;
};

#endif
