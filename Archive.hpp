#ifndef Archive_HPP
#define Archive_HPP
#include <QString>
#include <QCollator>
#include <QImageReader>

#include <memory>

class Archive {
public:
  Archive() : open_(false) {}
  virtual ~Archive() {}

  bool open(const QString& fileName);
  static bool test(const QString& fileName);

  QList<QString> getPathNameList() {
    return entryNameList_;
  }

  QByteArray getData(const QString& filename);

private:
  bool open_;
  QList<QString> entryNameList_;

  class Loader {
  public:
    Loader() {}
    virtual bool open(const QString &fileName) = 0;
    virtual void close() = 0;
    virtual QList<QString> getEntries() = 0;
    virtual QByteArray getData(const QString &filename) = 0;
    virtual ~Loader() {}
  };
  class UnarrLoader;
  class LibarchiveLoader;
  class DirectoryLoader;

  std::unique_ptr<Loader> loader_;
};

#endif
