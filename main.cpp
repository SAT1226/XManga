#include <QThreadPool>
#include <QApplication>
#include <QCoreApplication>

#include "XManga.hpp"

int main(int argc, char** argv)
{
  if (QThreadPool::globalInstance() -> maxThreadCount() < 3)
    QThreadPool::globalInstance() -> setMaxThreadCount(3);

  QApplication app(argc, argv);

  XManga win;
  win.show();

  QStringList args = QCoreApplication::arguments();
  args.removeAt(0);
  if(args.size() != 0) {
    win.open(args[0]);
  }

  return app.exec();
}
